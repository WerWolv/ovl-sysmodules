#include "gui_main.hpp"

#include "dir_iterator.hpp"

#include <json.hpp>
using json = nlohmann::json;

constexpr const char *const amsContentsPath = "/atmosphere/contents";
constexpr const char *const boot2FlagFormat = "/atmosphere/contents/%016lX/flags/boot2.flag";

static char pathBuffer[FS_MAX_PATH];

constexpr const char *const descriptions[2][2] = {
    [0] = {
        [0] = "Off | \uE098",
        [1] = "Off | \uE0F4",
    },
    [1] = {
        [0] = "On | \uE098",
        [1] = "On | \uE0F4",
    },
};

GuiMain::GuiMain() {
    Result rc = fsOpenSdCardFileSystem(&this->m_fs);
    if (R_FAILED(rc))
        return;

    FsDir contentDir;
    std::strcpy(pathBuffer, amsContentsPath);
    rc = fsFsOpenDirectory(&this->m_fs, pathBuffer, FsDirOpenMode_ReadDirs, &contentDir);
    if (R_FAILED(rc))
        return;
    tsl::hlp::ScopeGuard dirGuard([&] { fsDirClose(&contentDir); });

    /* Iterate over contents folder. */
    for (const auto &entry : FsDirIterator(contentDir)) {
        FsFile toolboxFile;
        std::snprintf(pathBuffer, FS_MAX_PATH, "/atmosphere/contents/%s/toolbox.json", entry.name);
        rc = fsFsOpenFile(&this->m_fs, pathBuffer, FsOpenMode_Read, &toolboxFile);
        if (R_FAILED(rc))
            continue;
        tsl::hlp::ScopeGuard fileGuard([&] { fsFileClose(&toolboxFile); });

        /* Get toolbox file size. */
        s64 size;
        rc = fsFileGetSize(&toolboxFile, &size);
        if (R_FAILED(rc))
            continue;

        /* Read toolbox file. */
        std::string toolBoxData(size, '\0');
        u64 bytesRead;
        rc = fsFileRead(&toolboxFile, 0, toolBoxData.data(), size, FsReadOption_None, &bytesRead);
        if (R_FAILED(rc))
            continue;

        /* Parse toolbox file data. */
        json toolboxFileContent = json::parse(toolBoxData);

        const std::string &sysmoduleProgramIdString = toolboxFileContent["tid"];
        u64 sysmoduleProgramId = std::strtoul(sysmoduleProgramIdString.c_str(), nullptr, 16);

        /* Let's not allow Tesla to be killed with this. */
        if (sysmoduleProgramId == 0x010000000007E51AULL)
            continue;

        SystemModule module = {
            .listItem = new tsl::elm::ListItem(toolboxFileContent["name"]),
            .programId = sysmoduleProgramId,
            .needReboot = toolboxFileContent["requires_reboot"],
        };

        module.listItem->setClickListener([this, module](u64 click) -> bool {
            if (click & KEY_A && !module.needReboot) {
                if (this->isRunning(module)) {
                    /* Kill process. */
                    pmshellTerminateProgram(module.programId);
                } else {
                    /* Start process. */
                    const NcmProgramLocation programLocation{
                        .program_id = module.programId,
                        .storageID = NcmStorageId_None,
                    };
                    u64 pid = 0;
                    pmshellLaunchProgram(0, &programLocation, &pid);
                }
                return true;
            }

            if (click & KEY_Y) {
                std::snprintf(pathBuffer, FS_MAX_PATH, boot2FlagFormat, module.programId);
                if (this->hasFlag(module)) {
                    /* Remove boot2 flag file. */
                    fsFsDeleteFile(&this->m_fs, pathBuffer);
                } else {
                    /* Create boot2 flag file. */
                    fsFsCreateFile(&this->m_fs, pathBuffer, 0, FsCreateOption(0));
                }
                return true;
            }

            return false;
        });
        this->m_sysmoduleListItems.push_back(std::move(module));
    }
    this->m_scanned = true;
}

GuiMain::~GuiMain() {
    fsFsClose(&this->m_fs);
}

tsl::elm::Element *GuiMain::createUI() {
    tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Sysmodules", VERSION);

    if (this->m_sysmoduleListItems.size() == 0) {
        const char *description = this->m_scanned ? "No sysmodules found!" : "Scan failed!";

        auto *warning = new tsl::elm::CustomDrawer([description](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE150", false, 180, 250, 90, renderer->a(0xFFFF));
            renderer->drawString(description, false, 110, 340, 25, renderer->a(0xFFFF));
        });

        rootFrame->setContent(warning);
    } else {
        tsl::elm::List *sysmoduleList = new tsl::elm::List();
        sysmoduleList->addItem(new tsl::elm::CategoryHeader("Dynamic  |  \uE0E0  Toggle  |  \uE0E3  Toggle auto start", true));
        sysmoduleList->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE016  These sysmodules can be toggled at any time.", false, x + 5, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
        }), 30);
        for (const auto &module : this->m_sysmoduleListItems) {
            if (!module.needReboot)
                sysmoduleList->addItem(module.listItem);
        }

        sysmoduleList->addItem(new tsl::elm::CategoryHeader("Static  |  \uE0E3  Toggle auto start", true));
        sysmoduleList->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("\uE016  These sysmodules need a reboot to work.", false, x + 5, y + 20, 15, renderer->a(tsl::style::color::ColorDescription));
        }), 30);
        for (const auto &module : this->m_sysmoduleListItems) {
            if (module.needReboot)
                sysmoduleList->addItem(module.listItem);
        }
        rootFrame->setContent(sysmoduleList);
    }

    return rootFrame;
}

void GuiMain::update() {
    static u32 counter = 0;

    if (counter++ % 20 != 0)
        return;

    for (const auto &module : this->m_sysmoduleListItems) {
        this->updateStatus(module);
    }
}

void GuiMain::updateStatus(const SystemModule &module) {
    bool running = this->isRunning(module);
    bool hasFlag = this->hasFlag(module);

    const char *desc = descriptions[running][hasFlag];
    module.listItem->setValue(desc);
}

bool GuiMain::hasFlag(const SystemModule &module) {
    FsFile flagFile;
    std::snprintf(pathBuffer, FS_MAX_PATH, boot2FlagFormat, module.programId);
    Result rc = fsFsOpenFile(&this->m_fs, pathBuffer, FsOpenMode_Read, &flagFile);
    if (R_SUCCEEDED(rc)) {
        fsFileClose(&flagFile);
        return true;
    } else {
        return false;
    }
}

bool GuiMain::isRunning(const SystemModule &module) {
    u64 pid = 0;
    if (R_FAILED(pmdmntGetProcessId(&pid, module.programId)))
        return false;

    return pid > 0;
}
