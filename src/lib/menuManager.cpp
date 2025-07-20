

#include "menuManager.h"
#include <ArduinoJson.h>
bool MenuManager::handleSensorMessage(sensor_message message)
{

    bool used = false;
    if (message.sensorType == SLIDER)
    {

        auto activeparams = getParametersForMenu(currentMenu);
        // debug for now
        int pindex = -1;
        auto name = getSensorName(message.sensorId);
        if (equalsIgnoreCase(name, "SLIDER1"))
            pindex = 0;
        else if (equalsIgnoreCase(name, "SLIDER2"))
            pindex = 1;
        else if (equalsIgnoreCase(name, "SLIDER3"))
            pindex = 2;
        else if (equalsIgnoreCase(name, "SLIDER4"))
            pindex = 3;
        else if (equalsIgnoreCase(name, "SLIDER5"))
            pindex = 4;
        if (pindex < 0 || pindex >= activeparams.size())
        {
            Serial.printf("Slider %s not found in active parameters %d of %d\n", name.c_str(), pindex, activeparams.size());

            return false; // disable slider for now
        }
        auto paramID = activeparams[pindex];
        auto paramName = getParameterName(paramID);

        Serial.printf("Set param %s %d from slider %s\n", paramName.c_str(), message.value, name.c_str());
        lastParameter.paramID = paramID;
        lastParameter.value = message.value;

        parameterChanged = true;

        return false; // disable slider for now

        // try
        // {
        //     for (int paramIndex = 0; paramIndex < activeparams.size(); paramIndex++)
        //     {

        //         if ((message.sensorId == SLIDER1 && paramIndex == 0) ||
        //             (message.sensorId == SLIDER2 && paramIndex == 1) ||
        //             (message.sensorId == SLIDER3 && paramIndex == 2) ||
        //             (message.sensorId == SLIDER4 && paramIndex == 3) ||
        //             (message.sensorId == SLIDER5 && paramIndex == 4))
        //         {

        //             auto paramID = activeparams[paramIndex];
        //             auto param = getIntParameter(paramID);

        //             int val = lerp(0, 255, param.min, param.max, message.value);
        //             // Serial.printf("Param[%d]  Slider: %d %s => %d\n", paramID, message.value, param.name.c_str(), val);

        //             if (val != param.value)
        //             {
        //                 if (isBoolParameter(paramID))
        //                 {
        //                     setBool(paramID, val != 0);
        //                 }
        //                 else
        //                 {
        //                     setValue(paramID, val);
        //                 }

        //                 meshManager->sendParametersToSlaves(paramID, val);
        //                 used = true;
        //             }
        //         }

        //         if (used)
        //         {
        //             updateMenu();
        //             return true;
        //         }
        //     }
        // }
        // catch (const std::exception &e)
        // {
        //     Serial.println("Error in Slider Processing");
        // }
    }
    else if (message.sensorType == DIAL)
    {
    }
    else if (message.sensorType == BUTTON)
    {
        int nummenus = numMenus();

#ifdef USE_AUDIO

        if (message.sensorId == BUTTON_TRIGGER)
        {
            audioManager->playTone(1000, 100, 5);
            meshManager->sendParametersToSlaves(PARAM_BEAT, 255);
            Serial.println("Trigger Button");
        }
        if (message.sensorId == BUTTON_LEFT)
        {
            audioManager->playTone(800, 100, 5);
        }
        if (message.sensorId == BUTTON_RIGHT)
        {
            audioManager->playTone(600, 100, 5);
        }
        if (message.sensorId == BUTTON_UP)
        {
            audioManager->playTone(400, 100, 5);
        }
        if (message.sensorId == BUTTON_DOWN)
        {
            audioManager->playTone(200, 100, 5);
        }

#endif
        if (message.value == 1)
        {

            if (menuMode == MENU_MODE_EDIT_MODE)
            {

                if (message.sensorId == BUTTON_LEFT)
                {

                    std::string menuName = getMenuName(currentMenu);
                    currentMenu = getParentMenu(currentMenu);
                    Serial.print("Leaving parameter Mode ");
                    Serial.println(menuName.c_str());
                    // verticalAlignment = true;
                    menuMode = MENU_MODE_MENU_CHOOSER;
                    // used = true;
                }

#ifdef USE_AUDIO
                if (message.sensorId == BUTTON_TRIGGER)
                {

                    if (currentMenu == MENU_AUDIO)
                    {
                        if (!audioManager->isRecordingAudio())
                        {
                            Serial.println("Recording Audio");
                            audioManager->record();
                        }
                        else
                        {
                            Serial.println("Playing Audio");
                            audioManager->stop();
                        }
                    }
                }
                if (message.sensorId == BUTTON_RIGHT)
                {
                    audioManager->play();
                }
                else if (message.sensorId == BUTTON_UP)
                {
                    audioManager->debugAudio();
                }
#endif
            }
            else if (menuMode == MENU_MODE_MENU_CHOOSER)
            {
                if (message.sensorId == BUTTON_UP)
                {
                    // up button
                    selectedMenu--;
                    if (selectedMenu < 0)
                    {
                        selectedMenu = nummenus - 1;
                        if (selectedMenu >= 4)
                        {
                            textOffset = selectedMenu - 4;
                        }
                        else
                        {
                            textOffset = 0;
                        }
                    }

                    auto smenus = getChildrenOfMenu(currentMenu);
                    if (selectedMenu < 0)
                    {
                        selectedMenu = smenus.size() - 1;
                    }

                    auto menu = smenus[selectedMenu];

                    used = true;
                }

                if (message.sensorId == BUTTON_DOWN)
                {

                    // down button
                    selectedMenu++;
                    if (selectedMenu >= 4)
                    {
                        textOffset = selectedMenu - 4;
                    }
                    else
                    {
                        textOffset = 0;
                    }

                    auto smenus = getChildrenOfMenu(currentMenu);
                    if (selectedMenu >= smenus.size())
                    {
                        selectedMenu = 0;
                    }
                    auto menu = smenus[selectedMenu];
                    Serial.printf("Selected Menu %d  %d %s\n", selectedMenu, menu, getMenuName(menu));

                    used = true;
                }
                if (message.sensorId == BUTTON_LEFT)
                {

                    // back button
                    Serial.println("Back Button");
                    if (currentMenu != MENU_IDLE)
                    {
                        currentMenu = getParentMenu(currentMenu);
                    }

                    used = true;
                }

                if (message.sensorId == BUTTON_RIGHT)
                {

                    auto availableMenus = getChildrenOfMenu(currentMenu);

                    if (selectedMenu >= availableMenus.size() || selectedMenu < 0)
                    {
                        if (availableMenus.size() == 0)
                        {
                            return false;
                        }
                        selectedMenu = 0;
                    }

                    currentMenu = availableMenus[selectedMenu];

                    std::string name = getMenuName(currentMenu, 32);

                    // meshManager->sendStringToSlaves("menu:" + String(name.c_str()));

                    if (getChildrenOfMenu(currentMenu).size() == 0) // if there are zero root menus for a mode, then go to edit mode
                    {
                        selectedMenu = 0;

                        // Serial.print("Menu Selected " + name + " " + String(currentMenu));
                        verticalAlignment = false;
                        menuMode = MENU_MODE_EDIT_MODE;
                    }
                }
                if (message.sensorId == BUTTON_TRIGGER)
                {
                    Serial.println("Trigger Button");
                }
                used = true;
            }
            else
            {
                Serial.printf("Not Handle Menu Mode %d %s\n", menuMode, getSensorName(message.sensorId).c_str());
            }

            if (used)
            {
                Serial.printf("Menu Changed: %s\n", getMenuName(currentMenu).c_str());

                return true;
            }
        }
    }
    return false;
}

void MenuManager::confirmMenus()
{
    //  this menu prints all the menus so the controller can confirm them
    //  should generate a json string that looks like this:

    Serial.println("Confirming Menus");
    JsonDocument doc;
    JsonObject root = doc.to<JsonObject>();
    for (const auto &menu : menuTypeMap)
    {
        JsonObject menuObj = root[getMenuName(menu.first)].to<JsonObject>();
        auto children = getChildrenOfMenu(menu.first);
        if (!children.empty())
        {
            for (const auto &child : children)
            {
                JsonArray params = menuObj[getMenuName(child)].to<JsonArray>();
                auto paramIDs = menuParameters.at(child);
                for (const auto &paramID : paramIDs)
                {
                    params.add(getParameterName(paramID));
                }
            }
        }
    }

    String jsonString;
    serializeJson(doc, jsonString);
    Serial.println();
}
std::vector<MenuID> MenuManager::getChildrenOfMenu(MenuID type)
{
    std::vector<MenuID> children;
    for (auto it = menuTypeMap.begin(); it != menuTypeMap.end(); ++it)
    {
        if (it->second.second == type)
        {
            children.push_back(it->first);
        }
    }
    return children;
}
std::string MenuManager::getMenuName(MenuID type, int MaxSize)
{
    auto it = menuTypeMap.find(type);
    if (it != menuTypeMap.end())
    {
        auto name = it->second.first;
        if (name.size() > MaxSize)
        {
            return name.substr(0, MaxSize);
        }
        return name;
    }
    return "UNKNOWN";
}

std::string MenuManager::getMenuPath(MenuID type, MenuID root)
{
    std::string path = getMenuName(type);
    MenuID parent = getParentMenu(type);
    while (parent != MENU_ROOT && parent != root)
    {
        path = std::string(getMenuName(parent)) + "/" + path;
        parent = getParentMenu(parent);
    }
    return path;
}
MenuID MenuManager::getParentMenu(MenuID type)
{
    auto it = menuTypeMap.find(type);
    if (it != menuTypeMap.end())
    {
        return it->second.second;
    }
    Serial.printf("Menu %d not found\n", type);
    return MENU_IDLE;
}

int MenuManager::numMenus()
{

    if (menuMode == MENU_MODE_MENU_CHOOSER)
    {

        return getChildrenOfMenu(currentMenu).size();
    }
    else if (menuMode == MENU_MODE_EDIT_MODE)
    {
        return getParametersForMenu(currentMenu).size();
    }
    return 0;
};
std::vector<ParameterID> MenuManager::getParametersForMenu(MenuID type)
{
    std::vector<ParameterID> params;
    for (auto it = menuParameters.begin(); it != menuParameters.end(); ++it)
    {
        if (it->first == type)
        {
            params = it->second;
            break;
        }
    }
    return params;
}
std::vector<std::string> MenuManager::getMenuItems()
{
    std::vector<std::string> menuText;
    if (currentMenu == MENU_IDLE)
    {
        menuText.push_back(".");
        return menuText;
    }

    auto availableMenus = getChildrenOfMenu(currentMenu);
    for (const auto &amenu : availableMenus)
    {
        std::string name = getMenuName(amenu, 8);
        menuText.push_back(name);
    }

    auto activeParams = getParametersForMenu(currentMenu);
    for (const auto &param : activeParams)
    {
        auto name = getParameterName(param);
        if (name.size() > 6)
        {
            name = name.substr(0, 6);
        }
        else if (name.size() < 6)
        {
            name = name + std::string(6 - name.size(), ' ');
        }
        if (isBoolParameter(param)) // TODO:: figure how to get the value
        {
            auto menuName = "p:" + name;
            menuText.push_back(menuName);
        }
        else
        {
            auto menuName = "p:" + name; // TODO:: figure how to get the value
            menuText.push_back(menuName);
        }
    }

    return menuText;

    // TODO: Implement the menu update logic

    // display.clearDisplay();
    // if (currentMenu == MENU_IDLE)
    // {
    //     display.setCursor(0, 0);
    //     display.println(".");
    //     display.display();
    //     return;
    // }

    // menuText.clear();

    // if (menuMode == MENU_MODE_MENU_CHOOSER)
    // {
    //     display.setRotation(0);

    //     auto availableMenus = getChildrenOfMenu(currentMenu);

    //     for (const auto &amenu : availableMenus)
    //     {

    //         std::string name = getMenuName(amenu, 8);

    //         menuText.push_back(name);
    //     }
    // }
    // else if (menuMode == MENU_MODE_EDIT_MODE)
    // {

    //     auto activeParams = getParametersForMenu(currentMenu);

    //     // const menu_item cmenu = menu[menuIndex];
    //     for (auto &param : activeParams)
    //     {
    //         auto name = getParameterName(param);
    //         if (name.size() > 6)
    //         {
    //             name = name.substr(0, 6);
    //         }
    //         else if (name.size() < 6)
    //         {
    //             name = name + std::string(6 - name.size(), ' ');
    //         }
    //         if (isBoolParameter(param))
    //         {
    //             auto menuName = name + ":" + (getBool(param) ? "ON" : "OFF");
    //             menuText.push_back(menuName.c_str());
    //         }
    //         else
    //         {
    //             auto menuName = name + ":" + std::to_string(getValue(param));
    //             menuText.push_back(menuName.c_str());
    //         }
    //     }

    //     display.setCursor(0, 0);
    //     int parent = currentMenu;
    //     // make sure menu path is at most 6 characters
    //     auto menupath = getMenuPath(currentMenu, MENU_PATTERNS);
    //     if (menupath.size() > 16)
    //     {
    //         menupath = menupath.substr(0, 16);
    //     }
    //     display.println(menupath.c_str());
    // }

    // for (int i = 0; i <= 8 && i < menuText.size(); i++)
    // {
    //     int xpos, ypos;
    //     int yoffset = 0;
    //     std::string text = menuText[i];
    //     if (verticalAlignment)
    //     {
    //         // Vertical alignment
    //         xpos = (i / 6) * 64; // Switch column after every 4 items
    //         ypos = (i % 6) * 8;  // Y position based on item index within the column
    //     }
    //     else
    //     {
    //         // Horizontal alignment
    //         xpos = (i % 2) * 64; // Two columns per row
    //         ypos = (i / 2) * 8;  // Four rows
    //         yoffset = 8;
    //     }
    //     display.setCursor(xpos, ypos + yoffset);

    //     // Check if the text is on the current line

    //     bool isCurrent = false;
    //     if (menuMode == MENU_MODE_MENU_CHOOSER)
    //     {
    //         isCurrent = i == selectedMenu;
    //     }
    //     else if (menuMode == MENU_MODE_EDIT_MODE)
    //     {
    //         isCurrent = false; // i == currentLine;
    //     }

    //     if (isCurrent)
    //     {
    //         display.fillRect(xpos, ypos + yoffset, 64, 8, WHITE); // Draw a white rectangle for highlighting
    //         display.setTextColor(BLACK);                          // Set text color to black for inversion
    //     }
    //     else
    //     {
    //         display.fillRect(xpos, ypos + yoffset, 64, 8, BLACK); // Draw a black rectangle for normal text
    //         display.setTextColor(WHITE);                          // Set text color to white for normal text
    //     }

    //     display.println(text.c_str());
    // }
    // display.display();
};
