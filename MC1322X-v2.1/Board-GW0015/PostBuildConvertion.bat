

"%~1\bin\objcopy.exe" --output-target=binary "%~2.out" "%~2.bin"
"%~1\bin\objcopy.exe" --gap-fill=0 --output-target=srec "%~2.out" "%~2.srec"
