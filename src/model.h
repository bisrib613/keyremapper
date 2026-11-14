#pragma once

constexpr int KRM_CODE_LIMIT = 0x1200;

enum ModMask : unsigned {
  KRM_SHIFT = 1,
  KRM_CTRL  = 2,
  KRM_ALT   = 4,
  KRM_WIN   = 8
};

struct Mapping {
  int source = 0;
  int target = 0;
  bool block = false;
  unsigned srcMods = 0;
  unsigned dstMods = 0;
};

struct Choice {
  int code = 0;
  unsigned mods = 0;
};
