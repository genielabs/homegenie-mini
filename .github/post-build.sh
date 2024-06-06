outputFolder="./artifacts";
if [ ! -d $outputFolder ]; then mkdir $outputFolder; fi;
pio project config | grep "env:" | while read -r line; do
  firmware="${line:4}";
  if [ -d "./.pio/build/${firmware}" ]; then
    cd ./.pio/build/
    zip "../../artifacts/${firmware}-${VERSION_NUMBER}.zip" "$firmware/firmware.bin" "$firmware/bootloader.bin" "$firmware/partitions.bin"
    cd ../..
  fi
done;
ls -latr artifacts/
