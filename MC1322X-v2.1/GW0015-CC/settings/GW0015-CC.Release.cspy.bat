@REM This batch file has been generated by the IAR Embedded Workbench
@REM C-SPY Debugger, as an aid to preparing a command line for running
@REM the cspybat command line utility using the appropriate settings.
@REM
@REM Note that this file is generated every time a new debug session
@REM is initialized, so you may want to move or rename the file before
@REM making changes.
@REM
@REM You can launch cspybat by typing the name of this batch file followed
@REM by the name of the debug file (usually an ELF/DWARF or UBROF file).
@REM
@REM Read about available command line parameters in the C-SPY Debugging
@REM Guide. Hints about additional command line parameters that may be
@REM useful in specific cases:
@REM   --download_only   Downloads a code image without starting a debug
@REM                     session afterwards.
@REM   --silent          Omits the sign-on message.
@REM   --timeout         Limits the maximum allowed execution time.
@REM 


"C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.0\common\bin\cspybat" "C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.0\arm\bin\armproc.dll" "C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.0\arm\bin\armjlink.dll"  %1 --plugin "C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.0\arm\bin\armbat.dll" --macro "C:\Users\Rafael Send\Documents\GitHub\FlyWeight\MC1322X-v2.1\GW0015-CC\..\PLM\FlashLoader\FlashMC1322x.mac" --flash_loader "C:\Program Files (x86)\IAR Systems\Embedded Workbench 7.0\arm\config\flashloader\" --backend -B "--endian=little" "--cpu=ARM7TDMI-S" "--fpu=None" "-p" "C:\Users\Rafael Send\Documents\GitHub\FlyWeight\MC1322X-v2.1\GW0015-CC\..\PLM\Icf\MC1322x.ddf" "--drv_verify_download" "--semihosting=none" "--drv_communication=USB0" "--jlink_speed=2000" 


