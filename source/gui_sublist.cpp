#include "gui_sublist.hpp"
#include "taunt_toggles.hpp"

class ClickableListItem : public tsl::elm::ListItem
{
public:
  ClickableListItem(std::string text, const std::vector<std::string> values, int* defaultPos, const std::string data, int index) : tsl::elm::ListItem(text), m_values(values), m_curValue(defaultPos), extdata(data)
  {
    this->index = index;
    this->setValue("");
  }

  ~ClickableListItem()
  {
  }

  tsl::elm::Element *requestFocus(Element *oldFocus, tsl::FocusDirection direction)
  {
    return ListItem::requestFocus(oldFocus, direction);
  }

  void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight)
  {
    ListItem::layout(parentX, parentY, parentWidth, parentHeight);
  }

  bool onClick(u64 keys)
  {
    if (keys & KEY_A)
    {
      if (this->m_valueChangeListener != nullptr) {
        this->m_valueChangeListener(this->m_values, this->m_curValue, this->extdata, this->index);
        return true;
      }
    }

    return false;
  }

  int getCurValue()
  {
    return *(this->m_curValue);
  }
  void setCurValue(int value) { *(this->m_curValue) = value; }

  const std::string getExtData() { return this->extdata; }

  const std::vector<std::string> getValues() { return this->m_values; }

  void setStateChangedListener(std::function<void(const std::vector<std::string>, int*, std::string, int index)> valueChangeListener) { this->m_valueChangeListener = valueChangeListener; }

private:
  const std::vector<std::string> m_values;
  int* m_curValue;
  std::function<void(const std::vector<std::string>, int*, std::string, int)> m_valueChangeListener = nullptr;
  const std::string extdata;
  int index;
};

GuiSublist::GuiSublist(std::vector<std::string> menuItems, int* index, std::string extData, std::string title) {
    this->menuItems = menuItems;
    this->index = index;
    this->extData = extData;
    this->title = title;
}

GuiSublist::~GuiSublist() {
}

tsl::elm::Element *GuiSublist::createUI() {
    tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame(title, "Select from the options below.");

    auto list = new tsl::elm::List();

    for (int i = 0; i < menuItems.size(); i++) {
        auto item = new ClickableListItem(menuItems[i], menuItems, this->index, "", i);
        item->setStateChangedListener([] (std::vector<std::string> a, int* b, std::string c, int index) {
            *b = index;
            tsl::goBack();
        });
        list->addItem(item);
        listItems.push_back(item);
    }

    list->setFocusedIndex(*index);

    rootFrame->setContent(list);

    return rootFrame;
}

void GuiSublist::update() {
    static u32 counter = 0;

    if (counter++ % 15 != 0)
        return;

    applyChanges();
}

void GuiSublist::applyChanges() {
}