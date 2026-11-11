using System.Runtime.InteropServices;
using System.Text.Json;
using System.ComponentModel;

namespace KeyRemapper;

internal static class Program
{
    [STAThread]
    private static void Main()
    {
        ApplicationConfiguration.Initialize();
        Application.Run(new RemapperForm());
    }
}

public sealed class Mapping
{
    public int Source { get; set; }
    public int Target { get; set; }
    public override string ToString() => $"{KeyLabel.Get(Source)}  →  {KeyLabel.Get(Target)}";
}

public sealed class Configuration
{
    public bool Enabled { get; set; }
    public List<Mapping> Mappings { get; set; } = new();
}

internal static class KeyLabel
{
    internal static readonly int[] Codes = Enumerable.Range(8, 247)
        .Where(k => (k >= 0x30 && k <= 0x5A) ||
                    (k >= 0x60 && k <= 0x87) ||
                    (k >= 0xA0 && k <= 0xA5) ||
                    (k >= 0xBA && k <= 0xC0) ||
                    (k >= 0xDB && k <= 0xDE) ||
                    new[] {8,9,13,16,17,18,19,20,27,32,33,34,35,36,37,38,39,40,44,45,46,91,92,93,144,145,226}.Contains(k))
        .ToArray();
    internal static string Get(int vk)
    {
        if (vk >= 0x30 && vk <= 0x5A) return ((char)vk).ToString();
        if (vk >= 0x70 && vk <= 0x87) return $"F{vk - 0x6F}";
        if (vk >= 0x60 && vk <= 0x69) return $"Num {vk - 0x60}";
        return ((Keys)vk).ToString();
    }
}

internal sealed class KeyboardEngine : IDisposable
{
    private const int WH_KEYBOARD_LL=13, WM_KEYDOWN=0x100, WM_KEYUP=0x101, WM_SYSKEYDOWN=0x104, WM_SYSKEYUP=0x105;
    private const uint KEYEVENTF_KEYUP=2, KEYEVENTF_EXTENDEDKEY=1, LLKHF_INJECTED=16;
    private IntPtr _hook;
    private readonly HookDelegate _callback;
    private readonly Dictionary<int,int> _held = new();
    internal Dictionary<int,int> Mappings {get;private set;} = new();
    internal bool Enabled {get;set;}
    internal bool Capturing {get;set;}
    internal event Action<int>? KeyCaptured;

    internal KeyboardEngine()
    {
        _callback=Hook;
        _hook=SetWindowsHookEx(WH_KEYBOARD_LL,_callback,IntPtr.Zero,0);
        if(_hook==IntPtr.Zero)
            throw new Win32Exception(Marshal.GetLastWin32Error(), "Keyboard hook could not be installed");
    }

    internal void SetMappings(IEnumerable<Mapping> mappings)
    {
        ReleaseHeld();
        Mappings=mappings.GroupBy(m=>m.Source).ToDictionary(g=>g.Key,g=>g.Last().Target);
    }
    private IntPtr Hook(int n,IntPtr wp,IntPtr lp)
    {
        if(n<0)return CallNextHookEx(_hook,n,wp,lp);
        var data=Marshal.PtrToStructure<KbdData>(lp);
        if((data.flags&LLKHF_INJECTED)!=0)return CallNextHookEx(_hook,n,wp,lp);
        bool down=wp==(IntPtr)WM_KEYDOWN||wp==(IntPtr)WM_SYSKEYDOWN;
        bool up=wp==(IntPtr)WM_KEYUP||wp==(IntPtr)WM_SYSKEYUP;
        if(!down&&!up)return CallNextHookEx(_hook,n,wp,lp);
        var code=(int)data.vkCode;
        if(Capturing)
        {
            if(down) KeyCaptured?.Invoke(code);
            return (IntPtr)1;
        }
        if(!Enabled || !Mappings.TryGetValue(code,out var target) || code==target)
            return CallNextHookEx(_hook,n,wp,lp);
        if(down && !_held.ContainsKey(code))
        {
            _held[code]=target;
            Send(target,false);
        }
        else if(up && _held.Remove(code,out var pressedTarget))
            Send(pressedTarget,true);
        return (IntPtr)1;
    }

    internal void ReleaseHeld()
    {
        foreach(var target in _held.Values)Send(target,true);
        _held.Clear();
    }

    private static void Send(int code,bool release)
    {
        uint flags=release?KEYEVENTF_KEYUP:0;
        if(code is 0x21 or 0x22 or 0x23 or 0x24 or 0x25 or 0x26 or 0x27 or 0x28
            or 0x2D or 0x2E or 0x5B or 0x5C or 0x6F or 0xA3 or 0xA5)
            flags|=KEYEVENTF_EXTENDEDKEY;
        INPUT[] inputs={new INPUT{type=1, u=new InputUnion{keyboard=new KeyboardInput{
            vk=(ushort)code,flags=flags,extra=(UIntPtr)0x4B52504D}}}};
        SendInput(1,inputs,Marshal.SizeOf<INPUT>());
    }

    public void Dispose()
    {
        ReleaseHeld();
        if(_hook!=IntPtr.Zero)UnhookWindowsHookEx(_hook);
        _hook=IntPtr.Zero;
    }
    private delegate IntPtr HookDelegate(int n,IntPtr wp,IntPtr lp);
    [StructLayout(LayoutKind.Sequential)]
    private struct KbdData {public uint code,scan,flags,time;public UIntPtr extra;}
    [StructLayout(LayoutKind.Sequential)]
    private struct INPUT {public uint type;public InputUnion u;}
    [StructLayout(LayoutKind.Explicit)]
    private struct InputUnion {[FieldOffset(0)]public KeyboardInput keyboard;}
    [StructLayout(LayoutKind.Sequential)]
    private struct KeyboardInput {public ushort vk,scan;public uint flags,time;public UIntPtr extra;}
    [DllImport("user32.dll",SetLastError=true)]private static extern IntPtr SetWindowsHookEx(int id,HookDelegate callback,IntPtr module,uint thread);
    [DllImport("user32.dll")]private static extern IntPtr CallNextHookEx(IntPtr hook,int n,IntPtr wp,IntPtr lp);
    [DllImport("user32.dll")]private static extern bool UnhookWindowsHookEx(IntPtr hook);
    [DllImport("user32.dll",SetLastError=true)]private static extern uint SendInput(uint count,INPUT[] input,int size);
}

public sealed class RemapperForm : Form
{
    private readonly KeyboardEngine _engine = new();
    private readonly List<Mapping> _mappings = new();
    private readonly BindingSource _binding = new();
    private readonly ComboBox _from=new(),_to=new();
    private readonly ListBox _list=new();
    private readonly Button _toggle=new();
    private readonly Label _status=new();
    private bool _captureFrom;
    private static readonly string ConfigPath=Path.Combine(
        Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),"KeyRemapper","settings.json");

    public RemapperForm()
    {
        Text="Key Remapper";
        ClientSize=new Size(740,530);
        MinimumSize=new Size(650,530);
        StartPosition=FormStartPosition.CenterScreen;
        BackColor=Color.FromArgb(21,25,33);
        ForeColor=Color.FromArgb(233,238,245);
        Font=new Font("Segoe UI",10);
        Controls.Add(new Label{Text="Key Remapper",Font=new Font("Segoe UI Semibold",21),
            Location=new Point(25,20),AutoSize=true});
        Controls.Add(new Label{Text="Assign a different function to any supported key",
            ForeColor=Color.Silver,Location=new Point(27,67),AutoSize=true});
        Controls.Add(new Label{Text="FROM",Location=new Point(27,112),AutoSize=true});
        Controls.Add(new Label{Text="TO",Location=new Point(378,112),AutoSize=true});
        SetupCombo(_from,new Point(27,140));
        SetupCombo(_to,new Point(378,140));
        Controls.AddRange(new Control[]{_from,_to});
        Button fromCapture=Button("Press key",229,139,125);
        Button toCapture=Button("Press key",578,139,125);
        fromCapture.Click+=(_,_)=>StartCapture(true);
        toCapture.Click+=(_,_)=>StartCapture(false);
        Button add=Button("Add mapping",27,196,150);
        add.Click+=(_,_)=>AddMapping();
        Button remove=Button("Remove selected",189,196,170);
        remove.Click+=(_,_)=>RemoveSelected();
        Button clear=Button("Restore defaults",371,196,180);
        clear.Click+=(_,_)=>{_mappings.Clear();Commit();SetEnabled(false);};
        _list.Location=new Point(27,252);
        _list.Size=new Size(676,180);
        _list.Anchor=AnchorStyles.Top|AnchorStyles.Left|AnchorStyles.Right|AnchorStyles.Bottom;
        _list.BackColor=Color.FromArgb(31,37,47);
        _list.ForeColor=ForeColor;
        _binding.DataSource=_mappings;
        _list.DataSource=_binding;
        Controls.Add(_list);
        _toggle.Text="Start remapping";
        _toggle.Location=new Point(27,455);
        _toggle.Size=new Size(165,39);
        _toggle.BackColor=Color.FromArgb(48,58,76);
        _toggle.ForeColor=ForeColor;
        _toggle.FlatStyle=FlatStyle.Flat;
        _toggle.Anchor=AnchorStyles.Left|AnchorStyles.Bottom;
        _toggle.Click+=(_,_)=>SetEnabled(!_engine.Enabled);
        Controls.Add(_toggle);
        Button save=Button("Export",204,455,100);
        Button load=Button("Import",316,455,100);
        save.Anchor=load.Anchor=AnchorStyles.Left|AnchorStyles.Bottom;
        save.Click+=(_,_)=>Export();
        load.Click+=(_,_)=>Import();
        _status.Location=new Point(440,462);
        _status.AutoSize=true;
        _status.Anchor=AnchorStyles.Left|AnchorStyles.Bottom;
        Controls.Add(_status);
        _engine.KeyCaptured+=vk=>
        {
            _engine.Capturing=false;
            if(vk==27){RefreshStatus();return;}
            var combo=_captureFrom?_from:_to;
            if(KeyLabel.Codes.Contains(vk))
            {
                for(int i=0;i<combo.Items.Count;i++)
                    if(combo.Items[i] is KeyOption option && option.Vk==vk)
                    { combo.SelectedIndex=i; break; }
            }
            RefreshStatus();
        };
        try {
            if(File.Exists(ConfigPath))Apply(JsonSerializer.Deserialize<Configuration>(
                File.ReadAllText(ConfigPath))??new Configuration());
            else SetEnabled(false);
        }
        catch(Exception ex){SetEnabled(false);MessageBox.Show(ex.Message,"Configuration error");}
        FormClosing+=(_,_)=>_engine.Dispose();
    }
    private void SetupCombo(ComboBox combo,Point position)
    {
        combo.Location=position;
        combo.Width=188;
        combo.DropDownStyle=ComboBoxStyle.DropDownList;
        combo.BackColor=Color.FromArgb(35,43,56);
        combo.ForeColor=ForeColor;
        combo.DataSource=KeyLabel.Codes.Select(x=>new KeyOption(x)).ToList();
        combo.DisplayMember="Title";
        combo.ValueMember="Vk";
    }
    private sealed record KeyOption(int Vk){public string Title=>KeyLabel.Get(Vk);}
    private Button Button(string title,int x,int y,int width)
    {
        var b=new Button{Text=title,Location=new Point(x,y),Size=new Size(width,36),
            BackColor=Color.FromArgb(47,57,74),ForeColor=ForeColor,FlatStyle=FlatStyle.Flat};
        Controls.Add(b);
        return b;
    }
    private void StartCapture(bool from)
    {
        _captureFrom=from;
        _status.Text="Press a key (Esc to cancel)";
        _engine.Capturing=true;
    }
    private void AddMapping()
    {
        if(_from.SelectedItem is not KeyOption from || _to.SelectedItem is not KeyOption to)return;
        if(from.Vk==to.Vk){MessageBox.Show("Choose two different keys.");return;}
        _mappings.RemoveAll(x=>x.Source==from.Vk);
        _mappings.Add(new Mapping{Source=from.Vk,Target=to.Vk});
        Commit();
    }
    private void RemoveSelected()
    {
        if(_list.SelectedItem is Mapping mapping){_mappings.Remove(mapping);Commit();}
    }
    private void Commit()
    {
        _engine.SetMappings(_mappings);
        _binding.ResetBindings(false);
        Persist();
        RefreshStatus();
    }
    private void SetEnabled(bool value)
    {
        if(!value)_engine.ReleaseHeld();
        _engine.Enabled=value;
        _toggle.Text=value?"Stop remapping":"Start remapping";
        _toggle.BackColor=value?Color.FromArgb(47,104,89):Color.FromArgb(48,58,76);
        Persist();
        RefreshStatus();
    }
    private void RefreshStatus()=>_status.Text=$"{(_engine.Enabled?"ON":"OFF")} · {_mappings.Count} mapping(s)";
    private Configuration Snapshot()=>new(){Enabled=_engine.Enabled,Mappings=_mappings};
    private void Persist()
    {
        Directory.CreateDirectory(Path.GetDirectoryName(ConfigPath)!);
        File.WriteAllText(ConfigPath,JsonSerializer.Serialize(Snapshot(),new JsonSerializerOptions{WriteIndented=true}));
    }
    private void Apply(Configuration config)
    {
        _engine.Enabled=false;
        _mappings.Clear();
        foreach(var item in config.Mappings??new())
            if(KeyLabel.Codes.Contains(item.Source) && KeyLabel.Codes.Contains(item.Target)
                && item.Source!=item.Target)
            {
                _mappings.RemoveAll(x=>x.Source==item.Source);
                _mappings.Add(item);
            }
        Commit();
        SetEnabled(config.Enabled);
    }
    private void Export()
    {
        using var dialog=new SaveFileDialog{Filter="JSON (*.json)|*.json",FileName="keyremapper.json"};
        if(dialog.ShowDialog()==DialogResult.OK)
            File.WriteAllText(dialog.FileName,JsonSerializer.Serialize(Snapshot(),new JsonSerializerOptions{WriteIndented=true}));
    }
    private void Import()
    {
        using var dialog=new OpenFileDialog{Filter="JSON (*.json)|*.json"};
        if(dialog.ShowDialog()==DialogResult.OK)
            try {Apply(JsonSerializer.Deserialize<Configuration>(File.ReadAllText(dialog.FileName))??new());}
            catch(Exception ex){MessageBox.Show(ex.Message,"Import failed");}
    }
}