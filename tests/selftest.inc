static LRESULT CALLBACK SelfTestWndProc(HWND h,UINT m,WPARAM w,LPARAM l) {
 if(m==WM_KR_CAPTURE) {
  gSelfCapturedCode=(int)w;
  gSelfCapturedMods=(unsigned)((ULONG_PTR)l>>1);
  return 0;
 }
 return DefWindowProcW(h,m,w,l);
}
struct TestEnvironmentCleanup {
 ~TestEnvironmentCleanup() {
  CleanupTestStartup();
  CleanupTestConfig();
 }
};
static int SelfTest() {
 TestEnvironmentCleanup cleanup;
  WNDCLASSEXW wc{sizeof(wc)};
 wc.hInstance=gInst; wc.lpfnWndProc=SelfTestWndProc; wc.lpszClassName=L"KRNativeSelfTest";
 RegisterClassExW(&wc);
 gWnd=CreateWindowExW(0,wc.lpszClassName,L"",WS_OVERLAPPED,0,0,715,525,nullptr,nullptr,gInst,nullptr);
 if(!gWnd) return 20;

 // Tray window lifecycle helpers: hide must keep the window/process alive, show must restore it.
 CreateUi(gWnd);
 ShowWindow(gWnd,SW_SHOW);
 HideDashboard();
 bool hideOk=!IsWindowVisible(gWnd);
 ShowDashboard();
 bool showOk=IsWindowVisible(gWnd)!=FALSE;
 if(!(hideOk&&showOk)) { DestroyWindow(gWnd); gWnd=nullptr; return 31; }

 // Left/right modifier state must survive releasing only one side.
 ResetPhysicalModifierState();
 UpdatePhysicalModifierState(VK_LSHIFT,true);
 UpdatePhysicalModifierState(VK_RSHIFT,true);
 UpdatePhysicalModifierState(VK_LSHIFT,false);
 bool dualShiftOk=(gPhysicalMods&KRM_SHIFT)!=0;
 UpdatePhysicalModifierState(VK_RSHIFT,false);
 dualShiftOk=dualShiftOk && (gPhysicalMods&KRM_SHIFT)==0;
 if(!dualShiftOk) { DestroyWindow(gWnd); gWnd=nullptr; return 39; }

 // Explicit owner-draw toggle state must survive without BM_GETCHECK.
 SetModChecks(KRM_SHIFT,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 bool modifierStateOk=(CheckedMods(gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin)&KRM_SHIFT)!=0;
 SetModChecks(0,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 if(!modifierStateOk) { DestroyWindow(gWnd); gWnd=nullptr; return 32; }

 // UI-selection test: resolve @ from the active keyboard layout, then Add/Update.
 Choice atChoice{};
 if(!ChoiceForSymbol(L'@',atChoice)) { DestroyWindow(gWnd); gWnd=nullptr; return 24; }
 SelectChoice(gSource,gSourceChoices,VK_OEM_3,0);
 SelectChoice(gTarget,gTargetChoices,atChoice.code,atChoice.mods);
 SetModChecks(0,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 SetModChecks(0,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 SetBlockSourceState(false);
 gMaps.clear();
 AddMap();
 wchar_t listTarget[64]{};
 if(ListView_GetItemCount(gList)>0) ListView_GetItemText(gList,0,1,listTarget,64);
 bool uiSelectionOk=(gMaps.size()==1 && gMaps[0].target==atChoice.code &&
                     gMaps[0].dstMods==atChoice.mods && wcscmp(listTarget,L"@")==0);
 if(!uiSelectionOk) { DestroyWindow(gWnd); gWnd=nullptr; return 24; }

 // Existing source mapping must be replaced by Add mapping, not silently rejected.
 gMaps.clear();
 gMaps.push_back({VK_OEM_3,'2',false,0,0});
 Refresh();
 SelectChoice(gSource,gSourceChoices,VK_OEM_3,0);
 SelectChoice(gTarget,gTargetChoices,atChoice.code,atChoice.mods);
 SetModChecks(0,gSrcShift,gSrcCtrl,gSrcAlt,gSrcWin);
 SetModChecks(0,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 SetBlockSourceState(false);
 AddMap();
 wchar_t upsertTarget[64]{};
 if(ListView_GetItemCount(gList)>0) ListView_GetItemText(gList,0,1,upsertTarget,64);
 bool upsertOk=(gMaps.size()==1 && gMaps[0].target==atChoice.code &&
                gMaps[0].dstMods==atChoice.mods && wcscmp(upsertTarget,L"@")==0);
 if(!upsertOk) { DestroyWindow(gWnd); gWnd=nullptr; return 28; }

 // Legacy migration regression: v1.6-style config must be rewritten after its probe handle is closed.
 Mapping legacyExpected=gMaps[0];
 DeleteFileW(ConfigBackupPath().c_str());
 DeleteFileW(ConfigTempPath().c_str());
 {
  std::wofstream legacy(ConfigPath(),std::ios::trunc);
  legacy<<legacyExpected.source<<L' '<<legacyExpected.target<<L' '<<(legacyExpected.block?1:0)
        <<L' '<<legacyExpected.srcMods<<L' '<<legacyExpected.dstMods<<L'\n';
 }
 gMaps.clear();
 gConfigLoadStatus=ConfigLoadStatus::Ready;
 bool migrationLoaded=Load();
 std::wstring migratedHeader;
 {
  std::wifstream migrated(ConfigPath());
  std::getline(migrated,migratedHeader);
 }
 bool migrationOk=migrationLoaded &&
                  gConfigLoadStatus==ConfigLoadStatus::Migrated &&
                  gMaps.size()==1 &&
                  gMaps[0].source==legacyExpected.source &&
                  gMaps[0].target==legacyExpected.target &&
                  gMaps[0].srcMods==legacyExpected.srcMods &&
                  gMaps[0].dstMods==legacyExpected.dstMods &&
                  migratedHeader==L"KRM_CONFIG 2";
 if(!migrationOk) { DestroyWindow(gWnd); gWnd=nullptr; return 40; }

 // Persistence regression: reload, recover corrupt primary, and preserve last-known-good backup.
 if(!Save()) { DestroyWindow(gWnd); gWnd=nullptr; return 29; }
 gMaps.clear();
 if(!Load()) { DestroyWindow(gWnd); gWnd=nullptr; return 29; }
 bool reloadOk=(gMaps.size()==1 && gMaps[0].target==atChoice.code && gMaps[0].dstMods==atChoice.mods);
 if(!reloadOk) { DestroyWindow(gWnd); gWnd=nullptr; return 29; }

 CopyFileW(ConfigPath().c_str(),ConfigBackupPath().c_str(),FALSE);
 {
  std::wofstream bad(ConfigPath(),std::ios::trunc);
  bad<<L"this is corrupt";
 }
 gMaps.clear();
 bool recovered=Load();
 bool recoveryOk=recovered && gMaps.size()==1 && gMaps[0].target==atChoice.code &&
                 gMaps[0].dstMods==atChoice.mods;
 if(!recoveryOk) { DestroyWindow(gWnd); gWnd=nullptr; return 30; }

 // A corrupt primary must never overwrite a healthy backup during the next save.
 {
  std::wofstream bad(ConfigPath(),std::ios::trunc);
  bad<<L"still corrupt";
 }
 gMaps.clear();
 gMaps.push_back({'C','D',false,0,0});
 if(!Save()) { DestroyWindow(gWnd); gWnd=nullptr; return 34; }
 std::vector<Mapping> backupMaps;
 bool backupPreserved=ReadMappingsFile(ConfigBackupPath(),backupMaps)==ConfigReadResult::Valid &&
                      backupMaps.size()==1 && backupMaps[0].target==atChoice.code &&
                      backupMaps[0].dstMods==atChoice.mods;
 if(!backupPreserved) { DestroyWindow(gWnd); gWnd=nullptr; return 34; }

 gMaps=backupMaps;
 WriteMappingsFile(ConfigPath(),gMaps);
 Refresh();

 // Modify regression: select the row, load it, change target to #, then save.
 Choice hashChoice{};
 if(!ChoiceForSymbol(L'#',hashChoice)) { DestroyWindow(gWnd); gWnd=nullptr; return 25; }
 ListView_SetItemState(gList,0,LVIS_SELECTED|LVIS_FOCUSED,LVIS_SELECTED|LVIS_FOCUSED);
 ModifySelected();
 bool modifyLoaded=(gEditingIndex==0);
 SelectChoice(gTarget,gTargetChoices,hashChoice.code,hashChoice.mods);
 SetModChecks(0,gDstShift,gDstCtrl,gDstAlt,gDstWin);
 AddMap();
 wchar_t modifiedTarget[64]{};
 if(ListView_GetItemCount(gList)>0) ListView_GetItemText(gList,0,1,modifiedTarget,64);
 bool modifyOk=modifyLoaded && gMaps.size()==1 && gMaps[0].target==hashChoice.code &&
               gMaps[0].dstMods==hashChoice.mods && wcscmp(modifiedTarget,L"#")==0;
 if(!modifyOk) { DestroyWindow(gWnd); gWnd=nullptr; return 25; }

 // Wheel regression: one 120 detent = one item; two 60 deltas = one item total.
 ComboBox_SetCurSel(gTarget,0); gTargetWheel.delta=0;
 int wheelStart=ComboBox_GetCurSel(gTarget);
 SendMessageW(gTarget,WM_MOUSEWHEEL,MAKEWPARAM(0,(WORD)-WHEEL_DELTA),0);
 bool wheelFull=(ComboBox_GetCurSel(gTarget)==wheelStart+1);
 ComboBox_SetCurSel(gTarget,0); gTargetWheel.delta=0;
 SendMessageW(gTarget,WM_MOUSEWHEEL,MAKEWPARAM(0,(WORD)-60),0);
 bool wheelHalfFirst=(ComboBox_GetCurSel(gTarget)==0);
 SendMessageW(gTarget,WM_MOUSEWHEEL,MAKEWPARAM(0,(WORD)-60),0);
 bool wheelHalfSecond=(ComboBox_GetCurSel(gTarget)==1);
 if(!(wheelFull&&wheelHalfFirst&&wheelHalfSecond)) {
  DestroyWindow(gWnd); gWnd=nullptr; return 26;
 }

 // Startup regression: toggle HKCU Run and restore the original state.
 bool startupInitial=StartupEnabled();
 bool startupOn=SetStartupEnabled(true) && StartupEnabled();
 bool startupCommandOk=StartupCommandValue().find(L"--startup")!=std::wstring::npos;
 bool startupOff=SetStartupEnabled(false) && !StartupEnabled();
 bool startupRestore=SetStartupEnabled(startupInitial);
 if(!(startupOn&&startupCommandOk&&startupOff&&startupRestore)) {
  DestroyWindow(gWnd); gWnd=nullptr; return 27;
 }
 gRunAtStartup=startupInitial; if(gStartup) InvalidateRect(gStartup,nullptr,TRUE);

 if(!InstallHooks()) { DestroyWindow(gWnd); return 21; }

 gSelfCapturedCode=0; gSelfCapturedMods=0; ResetPhysicalModifierState(); gCaptureSource=true; gCaptureTarget=false;
 SendTestKey(VK_SHIFT,true);
 SendTestKey('2',true);
 SendTestKey('2',false);
 SendTestKey(VK_SHIFT,false);

 DWORD until=GetTickCount()+1000;
 MSG msg;
 while(GetTickCount()<until && gSelfCapturedCode==0) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {
   TranslateMessage(&msg); DispatchMessageW(&msg);
  }
  Sleep(5);
 }
 auto isShiftVk=[](int vk){return vk==VK_SHIFT||vk==VK_LSHIFT||vk==VK_RSHIFT;};
  bool captureOk=(gSelfCapturedCode=='2' && (gSelfCapturedMods&KRM_SHIFT)!=0);
 if(!captureOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; return 22;
 }

 // Modifier-only capture must complete on modifier release without leaking the key to Windows.
 gSelfCapturedCode=0; gSelfCapturedMods=0; ResetPhysicalModifierState();
 gCaptureSource=true; gCaptureTarget=false; gCaptureModifierCandidate=0; gCaptureSuppressedMods=0;
 SendTestKey(VK_LSHIFT,true);
 SendTestKey(VK_LSHIFT,false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfCapturedCode==0) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 bool modifierCaptureOk=isShiftVk(gSelfCapturedCode) && gSelfCapturedMods==0;
 if(!modifierCaptureOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; return 35;
 }

 // Escape is a valid capturable key, not an implicit cancel key.
 gSelfCapturedCode=0; gSelfCapturedMods=0; ResetPhysicalModifierState();
 gCaptureSource=false; gCaptureTarget=true; gCaptureSwallow=0;
 SendTestKey(VK_ESCAPE,true);
 SendTestKey(VK_ESCAPE,false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfCapturedCode==0) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 if(gSelfCapturedCode!=VK_ESCAPE) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; return 36;
 }

 // Runtime emission test: A must produce Shift+2, not bare 2.
 gCaptureSource=gCaptureTarget=false; gCaptureSwallow=0; ResetPhysicalModifierState(); gPaused=false;
 gMaps.clear(); gMaps.push_back({'A','2',false,0,KRM_SHIFT});
 gSelfOutputEvents.clear(); gSelfObserveOutput=true;
 SendTestKey('A',true);
 SendTestKey('A',false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<4) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {
   TranslateMessage(&msg); DispatchMessageW(&msg);
  }
  Sleep(5);
 }
 gSelfObserveOutput=false;
 bool outputOk=false;
 for(size_t i=0;i+3<gSelfOutputEvents.size();++i) {
  if(isShiftVk(gSelfOutputEvents[i].vk) && gSelfOutputEvents[i].down &&
     gSelfOutputEvents[i+1].vk=='2' && gSelfOutputEvents[i+1].down &&
     gSelfOutputEvents[i+2].vk=='2' && !gSelfOutputEvents[i+2].down &&
     isShiftVk(gSelfOutputEvents[i+3].vk) && !gSelfOutputEvents[i+3].down) {
    outputOk=true; break;
  }
 }
 if(!outputOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; return 23;
 }

 // Disabling remapping while a mapped key is held must release its synthetic output.
 gMaps.clear(); gMaps.push_back({'B','2',false,0,KRM_SHIFT});
 gSelfOutputEvents.clear(); gSelfObserveOutput=true; gPaused=false;
 SendTestKey('B',true);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<2) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 SetRemappingEnabled(false);
 SendTestKey('B',false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<4) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 gSelfObserveOutput=false;
 bool releasedOk=gPaused && !gActive['B'].target;
 bool sawRelease=false;
 for(size_t i=0;i+3<gSelfOutputEvents.size();++i) {
  if(isShiftVk(gSelfOutputEvents[i].vk) && gSelfOutputEvents[i].down &&
     gSelfOutputEvents[i+1].vk=='2' && gSelfOutputEvents[i+1].down &&
     gSelfOutputEvents[i+2].vk=='2' && !gSelfOutputEvents[i+2].down &&
     isShiftVk(gSelfOutputEvents[i+3].vk) && !gSelfOutputEvents[i+3].down) {
    sawRelease=true; break;
  }
 }
 SetRemappingEnabled(true);
 if(!(releasedOk&&sawRelease)) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; return 33;
 }

 // A modifier key can be a source without requiring itself in srcMods.
 gMaps.clear(); gMaps.push_back({VK_LSHIFT,'A',false,0,0});
 ResetPhysicalModifierState(); gPaused=false; gSelfOutputEvents.clear(); gSelfObserveOutput=true;
 SendTestKey(VK_LSHIFT,true);
 SendTestKey(VK_LSHIFT,false);
 until=GetTickCount()+1000;
 while(GetTickCount()<until && gSelfOutputEvents.size()<2) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(5);
 }
 gSelfObserveOutput=false;
 bool modifierSourceOk=false;
 for(size_t i=0;i+1<gSelfOutputEvents.size();++i) {
  if(gSelfOutputEvents[i].vk=='A' && gSelfOutputEvents[i].down &&
     gSelfOutputEvents[i+1].vk=='A' && !gSelfOutputEvents[i+1].down) {
    modifierSourceOk=true; break;
  }
 }
 if(!modifierSourceOk) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; return 37;
 }

 // Releasing a key that was pressed while remapping was disabled must not synthesize a target.
 gMaps.clear(); gMaps.push_back({'C','D',false,0,0});
 SetRemappingEnabled(false); gSelfOutputEvents.clear(); gSelfObserveOutput=true;
 SendTestKey('C',true);
 until=GetTickCount()+100;
 while(GetTickCount()<until) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(2);
 }
 SetRemappingEnabled(true);
 SendTestKey('C',false);
 until=GetTickCount()+250;
 while(GetTickCount()<until) {
  while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) { TranslateMessage(&msg); DispatchMessageW(&msg); }
  Sleep(2);
 }
 gSelfObserveOutput=false;
 bool unexpectedTarget=false;
 for(const auto&e:gSelfOutputEvents) if(e.vk=='D') unexpectedTarget=true;
 if(unexpectedTarget) {
  Unhook(); DestroyWindow(gWnd); gWnd=nullptr; return 38;
 }

 Unhook();
 DestroyWindow(gWnd); gWnd=nullptr;  return 0;
}
