#include <tesla.hpp>

#include <fstream>
#include <map>
#include <filesystem>
#include <json.hpp>

using json = nlohmann::json;


bool isProgramRunning(u64 programId) {
    u64 pid = 0;
    if (R_FAILED(pmdmntGetProcessId(&pid, programId)))
        return false;
    
    return pid > 0;
}

class GuiMain : public tsl::Gui {
public:
    GuiMain() { 
        this->setTitle("Sysmodules");
        this->setSubtitle("v1.0.0");
    }
    
    ~GuiMain() { }

    virtual tsl::Element* createUI() {
        tsl::element::Frame *rootFrame = new tsl::element::Frame();
        tsl::element::List *sysmoduleList = new tsl::element::List();

        for (auto contentsFolder : std::filesystem::directory_iterator("sdmc:/atmosphere/contents")) {
            auto toolboxFile = contentsFolder.path() / "toolbox.json";

            if (std::filesystem::exists(toolboxFile)) {
                json toolboxFileContent;
                std::ifstream(toolboxFile) >> toolboxFileContent;

                std::string sysmoduleName = toolboxFileContent["name"];
                std::string sysmoduleProgramIdString = toolboxFileContent["tid"];
                u64 sysmoduleProgramId = strtoul(sysmoduleProgramIdString.c_str(), nullptr, 16);
                bool sysmoduleRequiresReboot = toolboxFileContent["requires_reboot"];

                if (sysmoduleRequiresReboot)
                    continue;
                
                // Let's not allow Tesla to be killed with this
                if (sysmoduleProgramId == 0x010000000007E51AULL)
                    continue;

                if (contentsFolder.path().string().find(sysmoduleProgramIdString) != std::string::npos) {

                    auto listEntry = new tsl::element::ToggleListItem(sysmoduleName, isProgramRunning(sysmoduleProgramId));
                    listEntry->setStateChangeListener([listEntry, sysmoduleProgramId](bool state) -> bool {
                        if (!isProgramRunning(sysmoduleProgramId)) {
                            const NcmProgramLocation programLocation {
                                .program_id = sysmoduleProgramId,
                                .storageID = NcmStorageId_None,
                            };
                            u64 pid = 0;

                            if (R_FAILED(pmshellLaunchProgram(0, &programLocation, &pid)))
                                listEntry->setState(false);
                        } else {
                            if (R_FAILED(pmshellTerminateProgram(sysmoduleProgramId)))
                                listEntry->setState(true);
                        }

                        return true;
                    });

                    this->m_sysmoduleListItems.insert({ sysmoduleProgramId, listEntry });
                    sysmoduleList->addItem(listEntry);
                }
            }
        }

        if (this->m_sysmoduleListItems.size() == 0) {
            auto warning = new tsl::element::CustomDrawer(0, 0, 100, FB_WIDTH, [](u16 x, u16 y, tsl::Screen *screen){
                screen->drawString("\uE150", false, 180, 250, 90, tsl::a(0xFFFF));
                screen->drawString("No sysmodules found!", false, 110, 340, 25, tsl::a(0xFFFF));
            });

            rootFrame->addElement(warning);
        }

        rootFrame->addElement(sysmoduleList);

        return rootFrame;
    }

    virtual void update() {
        static u32 counter = 0;

        if (counter++ % 20 != 0) return;

        for (const auto &[programId, listItem] : this->m_sysmoduleListItems) {
            listItem->setState(isProgramRunning(programId));
        }
    }

private:
    std::map<u64, tsl::element::ToggleListItem*> m_sysmoduleListItems;

};


class TemplateOverlay : public tsl::Overlay {
public:
    TemplateOverlay() { }
    ~TemplateOverlay() { }
    
    tsl::Gui* onSetup() { 
        smInitialize();
        pmshellInitialize();
        smExit();

        return new GuiMain();
    }

    void onDestroy() {
        pmshellExit();
    }

};


tsl::Overlay *overlayLoad() {
    return new TemplateOverlay();
}