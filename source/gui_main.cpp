#include "gui_main.hpp"
#include "taunt_toggles.h"

u64 pidSmash = 0;
static const char* SYSTEM_SETTINGS_FILE = "/atmosphere/config/system_settings.ini";
static const char* TRAINING_MOD_LOG = "/TrainingModpack/training_modpack.log";
static const char* TRAINING_MOD_CONF = "/TrainingModpack/training_modpack_menu.conf";

static tsl::hlp::ini::IniData readSettings() {
    /* Open Sd card filesystem. */
    FsFileSystem fsSdmc;
    if (R_FAILED(fsOpenSdCardFileSystem(&fsSdmc)))
        return {};
    tsl::hlp::ScopeGuard fsGuard([&] { fsFsClose(&fsSdmc); });

    /* Open config file. */
    FsFile fileConfig;
    if (R_FAILED(fsFsOpenFile(&fsSdmc, SYSTEM_SETTINGS_FILE, FsOpenMode_Read, &fileConfig)))
        return {};
    tsl::hlp::ScopeGuard fileGuard([&] { fsFileClose(&fileConfig); });

    /* Get config file size. */
    s64 configFileSize;
    if (R_FAILED(fsFileGetSize(&fileConfig, &configFileSize)))
        return {};

    /* Read and parse config file. */
    std::string configFileData(configFileSize, '\0');
    u64 readSize;
    Result rc = fsFileRead(&fileConfig, 0, configFileData.data(), configFileSize, FsReadOption_None, &readSize);
    if (R_FAILED(rc) || readSize != static_cast<u64>(configFileSize))
        return {};

    return tsl::hlp::ini::parseIni(configFileData);
}

static void writeSettings(tsl::hlp::ini::IniData const &iniData) {
    /* Open Sd card filesystem. */
    FsFileSystem fsSdmc;
    if (R_FAILED(fsOpenSdCardFileSystem(&fsSdmc)))
        return;
    tsl::hlp::ScopeGuard fsGuard([&] { fsFsClose(&fsSdmc); });

    std::string iniString = tsl::hlp::ini::unparseIni(iniData);

    fsFsDeleteFile(&fsSdmc, SYSTEM_SETTINGS_FILE);
    fsFsCreateFile(&fsSdmc, SYSTEM_SETTINGS_FILE, iniString.length(), 0);

    /* Open config file. */
    FsFile fileConfig;
    if (R_FAILED(fsFsOpenFile(&fsSdmc, SYSTEM_SETTINGS_FILE, FsOpenMode_Write, &fileConfig)))
        return;
    tsl::hlp::ScopeGuard fileGuard([&] { fsFileClose(&fileConfig); });

    fsFileWrite(&fileConfig, 0, iniString.c_str(), iniString.length(), FsWriteOption_Flush);
}

static void updateSettings(tsl::hlp::ini::IniData const &changes) {
    tsl::hlp::ini::IniData iniData = readSettings();
    for (auto &section : changes) {
        for (auto &keyValue : section.second) {
            iniData[section.first][keyValue.first] = keyValue.second;
        }
    }
    writeSettings(iniData);
}

GuiMain::GuiMain() {
    smInitialize();
    pminfoInitialize();
    pmbmInitialize();
    smExit();

    pmdmntGetProcessId(&pidSmash, 0x01006A800016E000);

    Result rc = fsOpenSdCardFileSystem(&this->m_fs);
    if (R_FAILED(rc))
        return;

    FsFile menuFile;
    rc = fsFsOpenFile(&this->m_fs, TRAINING_MOD_CONF, FsOpenMode_Read, &menuFile);
    if (R_FAILED(rc))
        return;

    u64 bytesRead;
    rc = fsFileRead(&menuFile, 0, static_cast<void *>(&menu), sizeof(menu), FsReadOption_None, &bytesRead);
    if (R_FAILED(rc)) {
        fsFileWrite(&menuFile, 0, static_cast<void *>(&menu), sizeof(menu), FsOpenMode_Write);
    }

    fsFileClose(&menuFile);
}

GuiMain::~GuiMain() {
    smInitialize();
    pminfoExit();
    pmbmExit();
    smExit();
}

tsl::elm::Element *GuiMain::createUI() {
    char buffer[256];
    snprintf(buffer, 256, "Version %s", VERSION);
    tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Training Modpack", buffer);

    auto list = new tsl::elm::List();

    Result rc;
    Handle debug;

    tsl::hlp::ini::IniData iniData = readSettings();
    bool ease_nro_restriction = false;
    for (auto &section : iniData) {
        for (auto &keyValue : section.second) {
            if (section.first == "ro") {
                if (keyValue.first == "ease_nro_restriction") {
                    ease_nro_restriction = (readSettings()["ro"]["ease_nro_restriction"] == "u8!0x1");
                }
            }
        }
    }

    if (!ease_nro_restriction) {
        tsl::elm::Element *iniShow = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawString(
                "Your config file did not have the \nproper configuration to run the \nTraining Modpack.\n\n\nIt has been automatically \nupdated.\n- atmosphere\n---- config\n-------- system_settings.ini\n\n(enable ease_nro_restriction)\n\n\nPlease reboot your Switch.", 
                false, 50, 225, 20, tsl::Color(255, 255, 255, 255));
        });

        updateSettings({
                { "ro", {
                    { "ease_nro_restriction", "u8!0x1" }
                }}
            });

        rootFrame->setContent(iniShow);
        return rootFrame;
    }

    if (pidSmash != 0) {
        rc = svcDebugActiveProcess(&debug, pidSmash);
        if (R_SUCCEEDED(rc)) {
          svcCloseHandle(debug);

          ValueListItem *hitboxItem = new ValueListItem("Hitbox Visualization", hitbox_items, menu.HITBOX_VIS, "hitbox");
          list->addItem(hitboxItem);
          valueListItems.push_back(hitboxItem);

          ValueListItem *shieldItem = new ValueListItem("Shield Options", shield_items, menu.SHIELD_STATE, "shield");
          list->addItem(shieldItem);
          valueListItems.push_back(shieldItem);

          ValueListItem *mashItem = new ValueListItem("Mash Toggles", mash_items, menu.MASH_STATE, "mash");
          list->addItem(mashItem);
          valueListItems.push_back(mashItem);

          ValueListItem *attackItem = new ValueListItem("Attack Toggles", attack_items, menu.ATTACK_STATE, "attack");
          list->addItem(attackItem);
          valueListItems.push_back(attackItem);

          ValueListItem *ledgeItem = new ValueListItem("Ledge Option", ledge_items, menu.LEDGE_STATE, "ledge");
          list->addItem(ledgeItem);
          valueListItems.push_back(ledgeItem);

          ValueListItem *techItem = new ValueListItem("Tech Options", tech_items, menu.TECH_STATE, "tech");
          list->addItem(techItem);
          valueListItems.push_back(techItem);

          ValueListItem *defensiveItem = new ValueListItem("Defensive Options", defensive_items, menu.DEFENSIVE_STATE, "defensive");
          list->addItem(defensiveItem);
          valueListItems.push_back(defensiveItem);

          ValueListItem *diItem = new ValueListItem("Set DI", di_items, menu.DI_STATE, "di");
          list->addItem(diItem);
          valueListItems.push_back(diItem);

          rootFrame->setContent(list);
        }
        else {
          tsl::elm::Element *warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawString("\uE150", false, 180, 250, 90, tsl::Color(255, 255, 255, 255));
            renderer->drawString("Could not debug process memory", false, 110, 340, 25, tsl::Color(255, 255, 255, 255));
          });

          rootFrame->setContent(warning);
        }
    } else {
        tsl::elm::Element *warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, u16 x, u16 y, u16 w, u16 h) {
            renderer->drawString("\uE150", false, 180, 250, 90, tsl::Color(255, 255, 255, 255));
            renderer->drawString("Smash not running.", false, 110, 340, 25, tsl::Color(255, 255, 255, 255));
        });

        rootFrame->setContent(warning);
    }

    return rootFrame;
}

void GuiMain::update() {
    static u32 counter = 0;

    if (counter++ % 20 != 0)
        return;

    applyChanges();
}

void GuiMain::applyChanges() {
    for (ValueListItem *item : valueListItems) {
        int value = item->getCurValue();
        std::string extData = item->getExtData();
        if (extData == "hitbox") menu.HITBOX_VIS = value;
        if (extData == "shield") menu.SHIELD_STATE = value;
        if (extData == "mash") menu.MASH_STATE = value;
        if (extData == "attack") menu.ATTACK_STATE = value;
        if (extData == "ledge") menu.LEDGE_STATE = value;
        if (extData == "tech") menu.TECH_STATE = value;
        if (extData == "defensive") menu.DEFENSIVE_STATE = value;
        if (extData == "di") menu.DI_STATE = value;
    }
    Result rc;
    Handle debug;

    if (pidSmash != 0) {
        rc = svcDebugActiveProcess(&debug, pidSmash);
        if (R_SUCCEEDED(rc)) {
            u64 menu_addr = 0;
            FsFile menuAddrFile;
            rc = fsFsOpenFile(&this->m_fs, TRAINING_MOD_LOG, FsOpenMode_Read, &menuAddrFile);
            if (R_FAILED(rc)) {
                svcCloseHandle(debug);
                return;
            }

            char buffer[100];
            u64 bytesRead;
            rc = fsFileRead(&menuAddrFile, 0, buffer, 100, FsReadOption_None, &bytesRead);
            if (R_FAILED(rc)) {
                svcCloseHandle(debug);
                return;
            }

            fsFileClose(&menuAddrFile);
            buffer[bytesRead] = '\0';
            menu_addr = strtoul(buffer, NULL, 16);

            if (menu_addr != 0) {
                rc = svcWriteDebugProcessMemory(debug, &menu, (u64)menu_addr, sizeof(menu));
            }
            svcCloseHandle(debug);
        }
    }

    FsFile menuFile;
    fsFsCreateFile(&this->m_fs, TRAINING_MOD_CONF, sizeof(menu), 0);

    rc = fsFsOpenFile(&this->m_fs, TRAINING_MOD_CONF, FsOpenMode_Write, &menuFile);
    if (R_FAILED(rc)) {
        fsFileClose(&menuFile);
        return;
    }

    rc = fsFileWrite(&menuFile, 0, static_cast<void *>(&menu), sizeof(menu), FsOpenMode_Write);
    if (R_FAILED(rc)) {
        fsFileClose(&menuFile);
        return;
    }

    fsFileClose(&menuFile);
}