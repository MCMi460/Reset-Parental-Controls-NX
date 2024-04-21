#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <switch.h>
#include <switch/services/pctl.h>
#include <assert.h>

bool debug;

typedef enum
{
    PctlSafteyLevel_None = 0x0,
    PctlSafteyLevel_Custom = 0x1,
    PctlSafteyLevel_YoungChild = 0x2,
    PctlSafteyLevel_Child = 0x3,
    PctlSafteyLevel_Teen = 0x4,
} PctlSafteyLevel;


// not tested
Result pctlConfirmLaunchApplicationPermission(uint64_t app_id)
{
    return serviceDispatchIn(pctlGetServiceSession_Service(), 1002, app_id);
}

// not tested
Result pctlConfirmResumeApplicationPermission(uint64_t app_id)
{
    return serviceDispatchIn(pctlGetServiceSession_Service(), 1003, app_id);
}

// not tested
Result pctlIsRestrictionTemporaryUnlocked(bool *out)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1006, *out);
}

// not tested
Result pctlIsRestrictedSystemSettingsEntered(bool *out)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1010, *out);
}

// works
Result pctlGetSafetyLevel(uint32_t *out)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1032, *out);
}

// works
Result pctlSetSafetyLevel(uint32_t level)
{
    return serviceDispatchIn(pctlGetServiceSession_Service(), 1033, level);
}

// works
Result pctlDeleteParentalControls(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1043);
}

// not tested
Result pctlNotifyApplicationDownloadStarted(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1047);
}

// doesn't work
Result pctlUnlockRestrictionTemporarily(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1201);
}

// works
Result pctlSetPinCode_Char(const char* pin)
{
    printf("LENGTH OF PIN: (%lu)\n", strlen(pin) + 1);
    printf("SIZE OF BUFFER: (%lu)\n", sizeof(pin));

    return serviceDispatch(pctlGetServiceSession_Service(), 1203,
        .buffer_attrs = {
            SfBufferAttr_HipcPointer | SfBufferAttr_In,
        },
        .buffers = { { pin, strlen(pin) + 1 } },
    );
}

// to maintain support with original(?)
// why did i bother with coding this
Result pctlSetPinCode(uint32_t pin)
{
    char send_pin[9];
    sprintf(send_pin, "%d", pin);
    assert(sizeof(send_pin) == 9);

    return pctlSetPinCode_Char(send_pin);
}

// works
Result pctlGetPinCodeLength(uint32_t *out)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1206, *out);
}

// doesn't work
Result pctlPostEnableAllFeatures(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1602);
}

// doesn't work
Result pctlIsAllFeaturesDisabled(bool *flag1, bool *flag2)
{
    struct {
        bool out0;
        bool out1;
    } out;

    Result r = serviceDispatchOut(pctlGetServiceSession_Service(), 1603, out);
    printf("\nsucc: %d\n", R_SUCCEEDED(r));
    if (R_SUCCEEDED(r)) {
        if (flag1) *flag1 = out.out0;
        if (flag2) *flag2 = out.out1;
    }
    return r;
}

// works
Result pctlDeletePairing(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1941);
}

// works
Result pctlStopPlayTimer(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1452);
}

// works
Result pctlStartPlayTimer(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1451);
}


// Huge thank you to @shadowninja108 on Github
typedef struct {
    char mData[0xA];
} PinCode;

// works
Result pctlGetPinCode(u32* lengthOut, PinCode *codeOut)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1208, *lengthOut,
        .buffer_attrs = { SfBufferAttr_HipcPointer | SfBufferAttr_Out },
        .buffers = { { codeOut, sizeof(*codeOut) } },
    );
}

typedef struct {
    u32 config[13];
} PlayTimerSettings;

// worksn't -- check later
Result pctlGetPlayTimerSettings(PlayTimerSettings *out)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1456, *out);
}

// works
Result pctlIsPairingActive(bool *flag)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1403, *flag);
}

// works
Result pctlIsPlayTimerEnabled(bool *flag)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1453, *flag);
}

void init_app(void)
{
    consoleInit(NULL);
    pctlInitialize();
}

void exit_app(void)
{
    consoleExit(NULL);
    pctlExit();
}

void print_display(const char *message)
{
    printf(message);
    consoleUpdate(NULL);
}

void show_menu(void) {
    print_display("\e[38;5;214mReset Parental Controls: Reseting the pin made easy!\e[0m\n\n\n");
    
    if (debug) print_display("\e[1;31mDEBUG MODE: ON\e[0m\n\n\n\n");

    print_display("Press (/\\) to clear the screen\n\n");
    if (debug) {
        print_display("Press (+) to disable debug mode\n\n");
        print_display("Press (-) to check the play timer status\n\n");
        print_display("Press (>) to check the pairing status\n\n");
        print_display("Press (Y) to check features disabled\n\n");
        print_display("Press (A) to edit the pin\n\n");
    } else {
        print_display("Press (ZL) to see the pin\n\n");
        print_display("Press (+) to enable debug mode\n\n");
        print_display("Press (L) to stop the play timer\n\n");
        print_display("Press (R) to start the play timer\n\n");
        print_display("Press (A) to set parental control pin\n\n");
        print_display("Press (X) to delete parental controls\n\n");
        print_display("Press (Y) to remove parental control app pairing\n\n");
    }
    print_display("Press (B) to exit\n\n");
}

void print_lock(const char *message)
{
    print_display(message);
    PadState pad;
    padInitializeDefault(&pad);
    while (appletMainLoop())
    {
        padUpdate(&pad);
        uint64_t k = padGetButtonsDown(&pad);
        if (k & HidNpadButton_B) break;
    }
}

/*
    USER METHODS
*/
void setParentalControls(void) {
    pctlExit();

    if (R_FAILED(pctlauthRegisterPasscode()))
    {
        print_display("\nFailed to change passcode\n");
    }
    else
    {
        print_display("\nParental control pin succesfully set!\n");
    }
    
    pctlInitialize();
}

void checkParentalPin(void) {
    if (hosversionBefore(4,0,0)) {
        printf("This method does not work on pre-4.0.0 consoles!\n");
        consoleUpdate(NULL);
        return;
    }
    PinCode pin = {};
    u32 length;
    Result r = pctlGetPinCode(&length, &pin);
    if(R_FAILED(r)) {
        printf("Failed (%x)\n", r);
        //printf("Length (%x)\n", length);
        //printf("Pin (%s)\n", pin.mData);
        consoleUpdate(NULL);
    } else {
        printf("\nSuccess (%x)\n", r);
        printf("Length (%x)\n", length);
        printf("Pin (%s)\n\n", pin.mData);
        consoleUpdate(NULL);
    }
}

void deleteParentalControls(void) {
    if (R_FAILED(pctlDeleteParentalControls()))
    {
        print_display("\nFailed to delete parental controls\n");
    }
    else
    {
        print_display("\nParental controls succesfully deleted!\n");
    }
}

void deleteAppPairing(void) {
    if (R_FAILED(pctlDeletePairing()))
    {
        print_display("\nFailed to remove app pairing\n");
    }
    else
    {
        print_display("\nRemoved app pairing!\n");
    }
}

void stopPlayTimer(void) {
    if (R_FAILED(pctlStopPlayTimer()))
    {
        print_display("\nFailed to stop play timer\n");
    }
    else
    {
        print_display("\nStopped play timer!\n");
    }
}

void startPlayTimer(void) {
    if (R_FAILED(pctlStartPlayTimer()))
    {
        print_display("\nFailed to start play timer\n");
    }
    else
    {
        print_display("\nStarted play timer!\n");
    }
}

/*
    DEBUG METHODS
*/
void setParentalControlsManually(PadState pad) {
    unsigned short length = 4;
    bool lenDone = false;
    while (!lenDone) {
        consoleClear();
        printf("...\n\nChoose length of new pin:\n\n");

        printf("_%d_ digits\n", length);

        consoleUpdate(NULL);
        padUpdate(&pad);
        uint64_t key = padGetButtonsDown(&pad);
        if (key & HidNpadButton_Up && length < 8) {
            length++;
        } else if (key & HidNpadButton_Down && length > 4) {
            length--;
        } else if (key & HidNpadButton_A) {
            lenDone = true;
        } else if (key & HidNpadButton_B) {
            break;
        }
    }
    if (!lenDone) {
        consoleClear();
        show_menu();
        return;
    }

    bool pinDone = false;
    char pin[9];
    
    sprintf(pin, "%0*d", length, 0);
    
    size_t index = 0;
    while (!pinDone) {
        consoleClear();
        printf("...\n\nChoose new pin:\n\n");

        for (size_t i = 0; i < length; i++) {
            if (i == index) printf("_%c_", pin[i]);
            else printf(" %c ", pin[i]);
        }

        consoleUpdate(NULL);
        padUpdate(&pad);
        uint64_t key = padGetButtonsDown(&pad);
        if (key & HidNpadButton_Up && pin[index] < 57) {
            pin[index]++;
        } else if (key & HidNpadButton_Down && pin[index] > 48) {
            pin[index]--;
        } else if (key & HidNpadButton_Left && index > 0) {
            index--;
        } else if (key & HidNpadButton_Right && index < length - 1) {
            index++;
        } else if (key & HidNpadButton_A) {
            pinDone = true;
        } else if (key & HidNpadButton_B) {
            break;
        }
    }
    if (!pinDone) {
        consoleClear();
        show_menu();
        return;
    }

    consoleClear();
    show_menu();

    Result r = pctlSetPinCode_Char(pin);
    if(R_FAILED(r)) {
        printf("Failed (%x)\n", r);
        printf("DESC (%d)\n", R_DESCRIPTION(r));
        consoleUpdate(NULL);
    } else {
        printf("Success (%x)\n", r);
        consoleUpdate(NULL);
    }
}

void checkFeaturesDisabled(void) {
    consoleUpdate(NULL);
    bool flag = false;
    bool flag2 = false;
    Result r = pctlIsAllFeaturesDisabled(&flag, &flag2);
    if(R_FAILED(r)) {
        printf("Failed (%x)\n", r);
        printf("DESC (%d)\n", R_DESCRIPTION(r));
        consoleUpdate(NULL);
    } else {
        printf("Success (%x)\n", r);
        printf("Data (%d)\n", (flag & 1));
        printf("Data2 (%d)\n", (flag2 & 1));
        consoleUpdate(NULL);
    }
}

void checkPlayTimerSettings(void) {
    printf("Testing...\n");
    consoleUpdate(NULL);
    PlayTimerSettings time = {};
    Result r = pctlGetPlayTimerSettings(&time);
    if(R_FAILED(r)) {
        printf("Failed (%x)\n", r);
        printf("Size (%lu)\n", sizeof(time));
        consoleUpdate(NULL);
    } else {
        printf("Success (%x)\n", r);
        printf("Size (%lu)\n", sizeof(time));
        printf("Data:\n");
        for (int i = 0; i < 13; i++) {
            printf("%u\n", time.config[i]);
        }
        consoleUpdate(NULL);
    }
}

void checkPlayTimer(void) {
    printf("Checking play timer status...\n");
    consoleUpdate(NULL);
    bool flag = false;
    Result r = pctlIsPlayTimerEnabled(&flag);
    if(R_FAILED(r)) {
        printf("Failed (%x)\n", r);
        consoleUpdate(NULL);
    } else {
        printf("Success (%x)\n", r);
        printf("Data (%d)\n", (flag & 1));
        consoleUpdate(NULL);
    }
}

void checkPairing(void) {
    printf("Checking pairing status...\n");
    consoleUpdate(NULL);
    bool flag = false;
    Result r = pctlIsPairingActive(&flag);
    if(R_FAILED(r)) {
        printf("Failed (%x)\n", r);
        consoleUpdate(NULL);
    } else {
        printf("Success (%x)\n", r);
        printf("Data (%d)\n", (flag & 1));
        consoleUpdate(NULL);
    }
}

int main(int argc, char *argv[])
{
    init_app();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    debug = false;
    show_menu();

    PadState pad;
    padInitializeDefault(&pad);

    while (appletMainLoop())
    {
        padUpdate(&pad);
        uint64_t k = padGetButtonsDown(&pad);

        if (k & HidNpadButton_A) {
            if (debug) {
                setParentalControlsManually(pad);
            } else {
                setParentalControls();
            }
            
        }

        if (k & HidNpadButton_ZL)
        {
            if (debug) {
                
            } else {
                checkParentalPin();
            }
        }

        if (k & HidNpadButton_L)
        {
            if (debug) {
                
            } else {
                stopPlayTimer();
            }
        }

        if (k & HidNpadButton_R)
        {
            if (debug) {
                
            } else {
                startPlayTimer();
            }
        }

        if (k & HidNpadButton_X)
        {
            if (debug) {
                
            } else {
                deleteParentalControls();
            }
        }

        if (k & HidNpadButton_Y)
        {
            if (debug) {
                checkFeaturesDisabled();
            } else {
                deleteAppPairing();
            }
        }

        if (k & HidNpadButton_ZR)
        {
            if (debug) {
                checkPlayTimerSettings();
            } else {
                
            }
        }

        if (k & HidNpadButton_Plus) {
            debug = !debug;
            consoleClear();
            printf("Toggling debug mode...\n\n");
            show_menu();
        }

        if (k & HidNpadButton_Minus) {
            if (debug) {
                checkPlayTimer();
            } else {

            }
        }

        if (k & HidNpadButton_Right) {
            if (debug) {
                checkPairing();
            } else {

            }
        }

        if (k & HidNpadButton_Up)
        {
            consoleClear();
            show_menu();
        }

        if (k & HidNpadButton_B)
        {
            break;
        }
    }

    exit_app();
    return 0;
}
