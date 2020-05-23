#pragma once

#include <list>
#include <tesla.hpp>

class GuiSublist : public tsl::Gui {
  private:
    std::vector<tsl::elm::ListItem *> listItems;
    std::vector<std::string> menuItems;
    int* index;
    std::string extData;
    std::string title;

  public:
    GuiSublist(std::vector<std::string> menuItems, int* index, std::string extData, std::string title);
    ~GuiSublist();

    virtual tsl::elm::Element *createUI();
    virtual void update() override;
    void applyChanges();
};