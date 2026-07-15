# Silicon Labs Si47xx (Silab FM)

This is a C++ implementation of the Silicon Labs **Si47xx (`silab fm`)** JNI wrapper:
- **JNI Interface:** implementation of `com/android/fmradio/FmNative` (`methodsRx`).
- **Internal Architecture:** Object-Oriented (`FmRadioController_silab` + `FmRadioRDSParser`) 

---

## File Structure:

| File | Description |
| :--- | :--- |
| **`FmRadioController_silab.h / .cpp`** | **OOP Radio Controller**. Encapsulates `/dev/radio0` file descriptors, power/tune/seek operations, mute handling (`DS_Mute`, `Int_Mute`), threshold controls (`RSSI`, `SNR`, `CNT`, `AF`), background pthread (`_ThreadHandleRDSData`) for RDS polling (`usleep(50000)` interval), and Alternative Frequency (`AF`) switching management. |
| **`FmRadioRDSParser.h / .cpp`** | **Thread-Safe RDS Parser**. Decodes Group `0A/0B` (Program Service `PS` name buffer filtering & validation), Group `2A/2B` (Radio Text `RT` decoding & new A/B flag handling), and VHF Band Alternative Frequency (`AF`) lists (`1..204` -> `87.6 + (code - 1) * 0.1` MHz). Protected via `pthread_mutex_t`. |
| **`FmNative_silab_jni.cpp`** | **Upgraded JNI Wrapper**. Implements all 16 methods of `com/android/fmradio/FmNative` (`openDev`, `powerUp`, `tune`, `seek`, `autoScan`, `stopScan`, `setRds`, `readRds`, `getPs`, `getLrText`, `activeAf`, `setMute`, `isRdsSupport`, `switchAntenna`) by delegating to a singleton `gRadioController` instance (`FmRadioController_silab*`). |
| **`silab_ioctl.h / .cpp`** | **Direct IOCTL Backend**. Defines exact ioctl constants (`SI47XX_IOC_POWERUP`, `SI47XX_IOC_CHAN_SELECT`, etc.) and data structures (`struct radio_data_t`, `struct sys_config2`, `struct sys_config3`) matching the Si47xx Linux kernel radio driver (`/dev/radio0`). |
| **`fmr.h / fmr_err.cpp`** | Common logging macros (`LOGI`, `LOGE`, `ALOG*`), error handling utilities, and standard POSIX/Android definitions. |
| **`Android.mk`** | Android NDK build makefile configured for `c++11`, compiling `libfmsilab_jni.so`. |
