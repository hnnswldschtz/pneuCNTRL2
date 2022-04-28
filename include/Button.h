#include "pneuCNTRL.h"


class Button {
  bool flag;
public:


  Button():flag(false)  {
}
  void setFlag()
  {
    flag = true;
  }

  bool getFlag()
  {
    return flag;
  }

  void clearFlag()
  {
    flag = false;
  }
};
