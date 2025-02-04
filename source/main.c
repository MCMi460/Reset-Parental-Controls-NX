#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <switch.h>


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

// doesn't work
Result pctlSetPinCode(uint32_t pin)
{
    return serviceDispatchIn(pctlGetServiceSession_Service(), 1203, pin);
}

// works
Result pctlGetPinCodeLength(uint32_t *out)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1206, *out);
}

/*
// doesn't work
Result pctlGetPinCode(uint32_t *out)
{
    return serviceDispatchOut(pctlGetServiceSession_Service(), 1208, *out);
}*/

// doesn't work
Result pctlPostEnableAllFeatures(void)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1602);
}

// works
Result pctlIsAllFeaturesDisabled(bool *out)
{
    return serviceDispatch(pctlGetServiceSession_Service(), 1603, *out);
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

int main(int argc, char *argv[])
{
    init_app();
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    print_display("Reset Parental Controls: Reseting the pin made easy!\n\n\n\n");

    print_display("Press (ZL) to see the pin\n\n");
    print_display("Press (L) to stop the play timer\n\n");
    print_display("Press (R) to start the play timer\n\n");
    print_display("Press (A) to set parental control pin\n\n");
    print_display("Press (X) to delete parental controls\n\n");
    print_display("Press (Y) to remove parental control app pairing\n\n");
    print_display("Press (B) to exit\n\n");

    PadState pad;
    padInitializeDefault(&pad);

    while (appletMainLoop())
    {
        padUpdate(&pad);
        uint64_t k = padGetButtonsDown(&pad);

        if (k & HidNpadButton_A)
        {
            pctlExit();

            if (R_FAILED(pctlauthRegisterPasscode()))
            {
                print_display("Failed to change passcode\n\n");
            }
            else
            {
                print_display("\n\n\n\nParental control pin succesfully set!\n\n\n\n");
            }
            
            pctlInitialize();
        }

        if (k & HidNpadButton_ZL)
        {

          printf("Testing...\n");
          consoleUpdate(NULL);
          PinCode pin = {};
          u32 length;
          Result r = pctlGetPinCode(&length, &pin);
          if(R_FAILED(r)) {
              printf("Failed (%x)\n", r);
              printf("Length (%x)\n", length);
              printf("Pin (%s)\n", pin.mData);
              consoleUpdate(NULL);
          } else {
            printf("Success (%x)\n", r);
            printf("Length (%x)\n", length);
            printf("Pin (%s)\n", pin.mData);
            consoleUpdate(NULL);
          }
        }

        if (k & HidNpadButton_L)
        {

            if (R_FAILED(pctlStopPlayTimer()))
            {
                print_display("Failed to stop play timer\n\n");
            }
            else
            {
                print_display("\n\n\n\nStopped play timer!\n\n\n\n");
            }
        }

        if (k & HidNpadButton_R)
        {

            if (R_FAILED(pctlStartPlayTimer()))
            {
                print_display("Failed to start play timer\n\n");
            }
            else
            {
                print_display("\n\n\n\nStarted play timer!\n\n\n\n");
            }
        }

        if (k & HidNpadButton_X)
        {
            if (R_FAILED(pctlDeleteParentalControls()))
            {
                print_display("Failed to delete parental controls\n\n");
            }
            else
            {
                print_display("\n\n\n\nParental controls succesfully deleted!\n\n\n\n");
            }
        }

        if (k & HidNpadButton_Y)
        {
            if (R_FAILED(pctlDeletePairing()))
            {
                print_display("Failed to remove app pairing\n\n");
            }
            else
            {
                print_display("\n\n\n\nRemoved app pairing!\n\n\n\n");
            }
        }

        if (k & HidNpadButton_B)
        {
            break;
        }
    }

    exit_app();
    return 0;
}
