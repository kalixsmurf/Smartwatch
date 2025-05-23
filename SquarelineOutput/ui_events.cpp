// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.5.1
// LVGL version: 8.3.11
// Project name: SquareLine_Project

#include "ui.h"
#include "ui_events.h"
#include <stdio.h>
#include "RTC_PCF85063.h"
#include "SD_Card.h"
#include <cstring>

static int prediction_row_counter = 1;

void CheckPassword(lv_event_t * e)
{
    const char* TAG = "LoginCheck";

    // Step 1: Get entered password (plain text)
    const char* enteredPassword = lv_textarea_get_text(ui_LoginPasswordTextArea);

    // Step 2: Open the credentials file
    File file = SD_MMC.open("/credentials.txt", FILE_READ);
    if (!file || !file.available()) {
        printf("[%s] Failed to open credentials.txt for reading\r\n", TAG);
        return;
    }

    // Step 3: Read up to the 3rd line (password)
    char line[128];
    int  lineCount     = 0;
    char savedPassword[64] = {0};

    while (file.available() && lineCount < 3) {
        size_t len = file.readBytesUntil('\n', line, sizeof(line));
        line[len] = '\0';
        // strip CR/LF
        while (len > 0 && (line[len-1]=='\n'||line[len-1]=='\r')) {
            line[--len] = '\0';
        }
        ++lineCount;
        if (lineCount == 3) {
            strncpy(savedPassword, line, sizeof(savedPassword)-1);
            break;
        }
    }
    file.close();

    // Step 4: Compare plain text
    if (strcmp(enteredPassword, savedPassword) == 0) {
        printf("[%s] Password correct. Proceeding...\r\n", TAG);
        _ui_screen_change(&ui_MainPage, LV_SCR_LOAD_ANIM_FADE_ON, 300, 0, &ui_MainPage_screen_init);
    } else {
        printf("[%s] Incorrect password!\r\n", TAG);
        lv_textarea_set_text(ui_LoginPasswordTextArea, "");
        lv_textarea_set_placeholder_text(ui_LoginPasswordTextArea, "Wrong Password");
    }
}

void SaveProfileInformationCallback(lv_event_t * e)
{
	SaveProfileInformation();
}

void LoadPredictionTableValues(lv_event_t * e)
{
    const char* TAG = "PredictionLoad";

    // STEP 1: Clear previously generated rows (skip the fixed header row)
    uint32_t child_count = lv_obj_get_child_cnt(ui_PredictionTable);
    for (uint32_t i = 1; i < child_count; i++) {
        lv_obj_t* child = lv_obj_get_child(ui_PredictionTable, i);
        lv_obj_del(child);  // Deletes the row and all its children (labels)
    }
    prediction_row_counter = 1;

    // STEP 2: Open file
    File file = SD_MMC.open("/results.txt", FILE_READ);
    if (!file || !file.available()) {
        printf("[%s] Failed to open results.txt\r\n", TAG);
        return;
    }

    char line[128];
    while (file.available()) {
        size_t len = file.readBytesUntil('\n', line, sizeof(line));
        line[len] = '\0';
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        char* time = strtok(line, ",");
        char* age = strtok(NULL, ",");
        char* gender = strtok(NULL, ",");
        char* emotion = strtok(NULL, ",");
        char* result = strtok(NULL, ",");

        if (!(time && age && gender && emotion && result)) {
            printf("[%s] Malformed line: %s\r\n", TAG, line);
            continue;
        }

        // STEP 3: Create new row
        lv_obj_t* row = lv_obj_create(ui_PredictionTable);
        lv_obj_remove_style_all(row);
        lv_obj_set_width(row, 280);
        lv_obj_set_height(row, LV_SIZE_CONTENT);
        lv_obj_set_align(row, LV_ALIGN_TOP_MID);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        //Debug row name
        printf("[%s] Creating row PredictionTableHeaderRow%d\n", TAG, prediction_row_counter++);

        auto create_cell = [&](const char* text, int width) {
            lv_obj_t* label = lv_label_create(row);
            lv_obj_set_width(label, width);
            lv_obj_set_height(label, LV_SIZE_CONTENT);
            lv_obj_set_align(label, LV_ALIGN_CENTER);
            lv_label_set_text(label, text);
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_color(label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_opa(label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
            lv_obj_set_style_border_width(label, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
        };

        create_cell(time, 56);
        create_cell(age, 40);
        create_cell(gender, 60);
        create_cell(emotion, 70);
        create_cell(result, 54);
    }

    file.close();
    printf("[%s] Finished loading predictions.\r\n", TAG);
}

void LoadProfileInformation(lv_event_t * e)
{
    const char* TAG = "ProfileLoad";

    File file = SD_MMC.open("/credentials.txt", FILE_READ);
    if (!file || !file.available()) {
        printf("[%s] Failed to open credentials.txt for reading\r\n", TAG);
        return;
    }

    // Buffers to read lines
    char line1[64] = {0};  // Primary
    char line2[64] = {0};  // Secondary
    char line3[64] = {0};  // Password

    // Read lines one by one
    file.readBytesUntil('\n', line1, sizeof(line1));
    file.readBytesUntil('\n', line2, sizeof(line2));
    file.readBytesUntil('\n', line3, sizeof(line3));
    file.close();

    // Remove any trailing carriage return or newline
    auto sanitize = [](char* str) {
        size_t len = strlen(str);
        while (len > 0 && (str[len - 1] == '\r' || str[len - 1] == '\n')) {
            str[len - 1] = '\0';
            len--;
        }
    };
    sanitize(line1);
    sanitize(line2);
    sanitize(line3);

    // Set as placeholder texts
    lv_textarea_set_placeholder_text(ui_PrimaryPhoneNumberInput, line1);
    lv_textarea_set_placeholder_text(ui_SecondaryPhoneNumberInput, line2);
    lv_textarea_set_placeholder_text(ui_ProfilePagePasswordInput, line3);

    printf("[%s] Profile info loaded from file\r\n", TAG);
}

void SaveSamplingRateToFile(lv_event_t * e)
{
    const char* TAG = "SamplingSave";
    File file = SD_MMC.open("/samplingRateConfig.txt", FILE_APPEND);
    if (!file) {
        printf("[%s] Failed to open samplingRateConfig.txt\r\n", TAG);
        return;
    }

    // Get selected time ranges into fixed buffers
    char from_time[32];
    char to_time[32];
    lv_dropdown_get_selected_str(ui_Dropdown1, from_time, sizeof(from_time));
    lv_dropdown_get_selected_str(ui_Dropdown2, to_time,   sizeof(to_time));

    // Determine the highest level checked
    const char* level = "None";
    if (lv_obj_has_state(ui_HighCheckbox1, LV_STATE_CHECKED)) {
        level = "High";
    } else if (lv_obj_has_state(ui_MediumCheckbox, LV_STATE_CHECKED)) {
        level = "Medium";
    } else if (lv_obj_has_state(ui_LowCheckbox, LV_STATE_CHECKED)) {
        level = "Low";
    }

    file.printf("%s,%s,%s\n", from_time, to_time, level);
    file.close();
    printf("[%s] Sampling config saved: %s,%s,%s\r\n", TAG, from_time, to_time, level);

    // Reset UI
    lv_dropdown_set_selected(ui_Dropdown1, 0);
    lv_dropdown_set_selected(ui_Dropdown2, 0);
    lv_obj_clear_state(ui_HighCheckbox1, LV_STATE_CHECKED);
    lv_obj_clear_state(ui_MediumCheckbox,   LV_STATE_CHECKED);
    lv_obj_clear_state(ui_LowCheckbox,      LV_STATE_CHECKED);

    SaveFilteringConfig();
}

void LoadSamplingRateValues(lv_event_t * e)
{
    const char* TAG = "SamplingLoad";

    // STEP 1: Remove previous rows (except header)
    uint32_t count = lv_obj_get_child_cnt(ui_SamplingRateTable);
    for (uint32_t i = 1; i < count; ++i) {
        lv_obj_del(lv_obj_get_child(ui_SamplingRateTable, 1)); // Always delete second item (index 1), as the list shifts
    }

    // STEP 2: Open the file
    File file = SD_MMC.open("/samplingRateConfig.txt", FILE_READ);
    if (!file || !file.available()) {
        printf("[%s] Failed to open samplingRateConfig.txt\r\n", TAG);
        return;
    }

    char line[128];
    while (file.available()) {
        size_t len = file.readBytesUntil('\n', line, sizeof(line));
        line[len] = '\0';

        // Strip CRLF
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
            line[--len] = '\0';
        }

        // Parse line
        char* from_time = strtok(line, ",");
        char* to_time = strtok(NULL, ",");
        char* level = strtok(NULL, ",");

        if (!(from_time && to_time && level)) {
            printf("[%s] Skipping malformed line: %s\n", TAG, line);
            continue;
        }

        // Format: "13:00 to 15:00"
        char time_range[64];
        snprintf(time_range, sizeof(time_range), "%s to %s", from_time, to_time);

        // STEP 3: Create row container
        lv_obj_t* row = lv_obj_create(ui_SamplingRateTable);
        lv_obj_remove_style_all(row);
        lv_obj_set_width(row, 280);
        lv_obj_set_height(row, LV_SIZE_CONTENT);
        lv_obj_set_align(row, LV_ALIGN_TOP_MID);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
        lv_obj_set_flex_align(row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
        lv_obj_clear_flag(row, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

        // Time column
        lv_obj_t* time_label = lv_label_create(row);
        lv_obj_set_width(time_label, 140);
        lv_obj_set_height(time_label, LV_SIZE_CONTENT);
        lv_obj_set_align(time_label, LV_ALIGN_CENTER);
        lv_label_set_text(time_label, time_range);
        lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(time_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(time_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(time_label, 2, LV_PART_MAIN | LV_STATE_DEFAULT);

        // Level column
        lv_obj_t* level_label = lv_label_create(row);
        lv_obj_set_width(level_label, 140);
        lv_obj_set_height(level_label, LV_SIZE_CONTENT);
        lv_obj_set_align(level_label, LV_ALIGN_CENTER);
        lv_label_set_text(level_label, level);
        lv_obj_set_style_text_align(level_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_color(level_label, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_opa(level_label, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(level_label, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    }

    file.close();
    printf("[%s] Sampling rate table populated successfully.\r\n", TAG);
}

void UpdateLockScreenTime(lv_event_t * e)
{
    char time_str[6];  // "HH:MM"
    snprintf(time_str, sizeof(time_str), "%02d:%02d", datetime.hour, datetime.minute);

    // Update the label
    if (ui_LockPageTimeLabel != NULL) {
        lv_label_set_text(ui_LockPageTimeLabel, time_str);
    }
}

void ClearSamplingRateTable(lv_event_t * e)
{
    const char* TAG = "SamplingClear";

    // STEP 1: Delete all rows except header (index 0)
    uint32_t count = lv_obj_get_child_cnt(ui_SamplingRateTable);
    for (uint32_t i = 1; i < count; ++i) {
        lv_obj_del(lv_obj_get_child(ui_SamplingRateTable, 1)); // Always delete the second item
    }

    printf("[%s] Cleared all sampling rate table rows (except header).\r\n", TAG);

    // STEP 2: Truncate the file by reopening in FILE_WRITE mode (no write = empty)
    File file = SD_MMC.open("/samplingRateConfig.txt", FILE_WRITE);
    if (!file) {
        printf("[%s] Failed to clear samplingRateConfig.txt\r\n", TAG);
        return;
    }
    file.close();

    printf("[%s] samplingRateConfig.txt content cleared successfully.\r\n", TAG);
}

void SaveProfileInformation()
{
    const char* TAG = "ProfileSave";

    const char* primary    = lv_textarea_get_text(ui_PrimaryPhoneNumberInput);
    const char* secondary  = lv_textarea_get_text(ui_SecondaryPhoneNumberInput);
    const char* rawPassword = lv_textarea_get_text(ui_ProfilePagePasswordInput);

    // Overwrite the credentials file with plain text
    File file = SD_MMC.open("/credentials.txt", FILE_WRITE);
    if (!file) {
        printf("[%s] Failed to open credentials.txt for writing\r\n", TAG);
        return;
    }

    file.printf("%s\n", primary);
    file.printf("%s\n", secondary);
    file.printf("%s\n", rawPassword);

    file.close();
    printf("[%s] Credentials saved successfully\r\n", TAG);
}

void SaveFilteringConfig()
{
    const char* TAG = "FilterSave";
    File file = SD_MMC.open("/filteringConfig.txt", FILE_WRITE);
    if (!file) {
        printf("[%s] Failed to open filteringConfig.txt\r\n", TAG);
        return;
    }

    auto write_cb = [&](lv_obj_t* cb, const char* label) {
        int state = lv_obj_has_state(cb, LV_STATE_CHECKED) ? 1 : 0;
        file.printf("%s,%d\n", label, state);
    };

    // Gender
    write_cb(ui_MaleCheckbox, "Male");
    write_cb(ui_FemaleCheckbox, "Female");

    // Emotions
    write_cb(ui_AngryCheckBox, "Angry");
    write_cb(ui_SadCheckBox, "Sad");
    write_cb(ui_NeutralCheckBox, "Neutral");
    write_cb(ui_CalmCheckBox, "Calm");
    write_cb(ui_HappyCheckBox, "Happy");
    write_cb(ui_FearCheckBox, "Fear");
    write_cb(ui_DisgustCheckBox, "Disgust");
    write_cb(ui_SurprizedCheckBox, "Surprised");

    // Ages
    write_cb(ui_twentyCheckbox, "20s");
    write_cb(ui_thirtyCheckbox, "30s");
    write_cb(ui_fortyCheckbox, "40s");
    write_cb(ui_fiftyCheckbox, "50s");
    write_cb(ui_sixtyCheckbox, "60s");
    write_cb(ui_seventyCheckbox, "70s");
    write_cb(ui_eightyCheckbox, "80s");

    file.close();
    printf("[%s] Config saved to filteringConfig.txt\r\n", TAG);
}
