
#pragma once
#include "shared.h"
#include "menus.h"
#include <Arduino.h>
#include "string_utils.h"
enum MenuMode
{

    MENU_MODE_MENU_CHOOSER,
    MENU_MODE_EDIT_MODE,
};

typedef struct param_change
{
    ParameterID paramID;
    int value;
} param_change;
class MenuManager
{
private:
    MenuID currentMenu;
    MenuMode menuMode;
    int selectedMenu;
    int textOffset = 0;
    bool verticalAlignment = true; // Set this flag to true for vertical alignment, false for horizontal
    bool parameterChanged = false;
    bool menuChanged = false;
    param_change lastParameter;

public:
    MenuManager()
        : currentMenu(MENU_ROOT), menuMode(MENU_MODE_MENU_CHOOSER), selectedMenu(0)
    {

        Serial.println("MenuManager initialized");
    }
    bool messageAvailable() const
    {
        return parameterChanged;
    }
    bool isMenuChanged()
    {
        auto changed = menuChanged;
        menuChanged = false; // Reset after checking
        return changed;
    }
    param_change getMessage()
    {
        parameterChanged = false;
        return lastParameter;
    }
    bool handleSensorMessage(sensor_message message);
    bool handleTextMessage(std::string message)
    {
        if (message.find("confirmmenu") == 0)
        {
            confirmMenus();
            return true;
        }

        if (message.find("menu") == 0)
        {
            // search for : <menu_name>
            if (message.find(':') == std::string::npos)
            {
                printMenu();
                return true;
            }

            std::string menuName = message.substr(5);
            trim(menuName);

            if (equalsIgnoreCase(menuName, "root"))
            {
                currentMenu = MENU_ROOT;
                selectedMenu = 0; // Reset selected menu
                menuChanged = true;
                printMenu();
                return true;
            }
            if (equalsIgnoreCase(menuName, "up"))
            {
                currentMenu = getParentMenu(currentMenu);
                selectedMenu = 0; // Reset selected menu
                menuChanged = true;
                printMenu();
                return true;
            }
            if (equalsIgnoreCase(menuName, "all"))
            {
                for (const auto &menu : menuTypeMap)
                {
                    Serial.println(menu.second.first.c_str());
                }

                return true;
            }

            auto menuID = getMenuByName(menuName);
            //  find in the list of menus
            if (menuID != MENU_IDLE)
            {
                currentMenu = menuID;
                selectedMenu = 0; // Reset selected menu
                menuChanged = true;
                printMenu();
                return true;
            }
            Serial.printf("Menu %s not found\n", menuName.c_str());
        }
        return false;
    }
    std::vector<MenuID> getChildrenOfMenu(MenuID type);
    std::string getMenuName(MenuID type, int MaxSize = 6);
    void selectMenu(MenuID type)
    {
        currentMenu = type;
        selectedMenu = 0; // Reset selected menu
        menuChanged = true;
        Serial.printf("Selected menu: %s\n", getMenuName(type).c_str());
    }
    std::string getMenuPath(MenuID type, MenuID root);
    MenuID getParentMenu(MenuID type);
    MenuID getMenuByName(const std::string &name)
    {
        // first look in the current menus
        auto children = getChildrenOfMenu(currentMenu);
        for (const auto &child : children)
        {
            if (equalsIgnoreCase(getMenuName(child), name))
            {
                return child;
            }
        }
        for (const auto &menu : menuTypeMap)
        {
            if (startsWith(menu.second.first, name))
            {
                return menu.first;
            }
        }
        return MENU_IDLE; // Return a default value if not found
    }
    void printMenu()
    {
        auto items = getMenuItems();

        for (const auto &item : items)
        {
            Serial.println(item.c_str());
        }
    }
    std::vector<ParameterID> getParametersForMenu(MenuID type);
    int numMenus();
    void confirmMenus();
    std::vector<std::string> getMenuItems();
};