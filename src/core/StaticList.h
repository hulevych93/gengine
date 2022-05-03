#pragma once

#include <boost/format.hpp>

namespace Gengine {

template <class Action>
class FactoryItem {
  typedef void (Action::*_Proc_t)();

 public:
  FactoryItem(const Action& _Reg);

  void Register() { _Do(&Action::Do, false, true); }

  void Unregister() { _Do(&Action::Undo, true, false); }

 private:
  void _Do(_Proc_t proc, bool pattern, bool complete) {
    if (succeeded == pattern) {
      (reg.*proc)();
      succeeded = complete;
    }
  }

 public:
  Action reg;
  FactoryItem* next;
  bool succeeded;
};

template <class Action>
struct FactoriesList {
  typedef FactoryItem<Action> FI;

  FactoriesList() : head(nullptr), succeeded(false) {}

  void Push(FI* item) {
    if (succeeded)
      succeeded = false;

    item->next = head;
    head = item;
  }

  static FactoriesList& Instance() {
    static FactoriesList instance;
    return instance;
  }

  FI* head;
  bool succeeded;
};

template <class Action>
FactoryItem<Action>::FactoryItem(const Action& _Reg)
    : next(nullptr), reg(_Reg), succeeded(false) {
  FactoriesList<Action>::Instance().Push(this);
}

template <class Action>
class Runtime {
  typedef FactoryItem<Action> _Fi_t;
  typedef FactoriesList<Action> _Fl_t;
  typedef void (_Fi_t::*_Proc_t)();

 public:
  static void Do() { _Do(&_Fi_t::Register, false, true); }

  static void Undo() { _Do(&_Fi_t::Unregister, true, false); }

 private:
  static void _Do(_Proc_t proc, bool pattern, bool complete) {
    if (_Fl().succeeded == pattern) {
      _Apply(proc, pattern, complete);
      _Fl().succeeded = complete;
    }
  }

  static void _Apply(_Proc_t proc, bool pattern, bool complete) {
    for (auto fi = _Fl().head; fi != nullptr; fi = fi->next) {
      if (fi->succeeded == pattern) {
        (fi->*proc)();
        fi->succeeded = complete;
      }
    }
  }

  static _Fl_t& _Fl() { return _Fl_t::Instance(); }
};

}  // namespace Gengine
