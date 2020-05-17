#pragma once

#include <list>
#include <tesla.hpp>

struct SystemModule {
    tsl::elm::ListItem *listItem;
    u64 programId;
    bool needReboot;
};

class ValueListItem : public tsl::elm::ListItem
{
public:
  ValueListItem(std::string text, const std::vector<std::string> values, int defaultPos, const std::string data) : tsl::elm::ListItem(text), m_values(values), m_curValue(defaultPos), extdata(data)
  {
    this->setValue(m_values[m_curValue]);
  }

  ~ValueListItem()
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
      m_curValue++;
      long int size = m_values.size();
      if (m_curValue < 0)
        m_curValue = size - 1;
      if (size <= m_curValue)
        m_curValue = 0;

      this->setValue(m_values[m_curValue]);

      if (this->m_valueChangeListener != nullptr)
        this->m_valueChangeListener(this->m_values, this->m_curValue, this->extdata);

      return true;
    }

    return false;
  }

  int getCurValue()
  {
    return this->m_curValue;
  }
  void setCurValue(int value) { this->m_curValue = value; }

  const std::string getExtData() { return this->extdata; }

  const std::vector<std::string> getValues() { return this->m_values; }

  void setStateChangedListener(std::function<void(const std::vector<std::string>, int, std::string)> valueChangeListener) { this->m_valueChangeListener = valueChangeListener; }

private:
  const std::vector<std::string> m_values;
  int m_curValue;
  std::function<void(const std::vector<std::string>, int, std::string)> m_valueChangeListener = nullptr;
  const std::string extdata;
};




class GuiMain : public tsl::Gui {
  private:
    FsFileSystem m_fs;
    std::list<SystemModule> m_sysmoduleListItems;
    bool m_scanned;
    tsl::elm::ToggleListItem *toggleItem = nullptr;
    std::vector<ValueListItem *> valueListItems;

  public:
    GuiMain();
    ~GuiMain();

    virtual tsl::elm::Element *createUI();
    virtual void update() override;
    void applyChanges();

  private:
    void updateStatus(const SystemModule &module);
    bool hasFlag(const SystemModule &module);
    bool isRunning(const SystemModule &module);
};