#! /bin/bash


FONTS=(
  cmr:ComputerModern-Regular:0
  cmbx:ComputerModern-Bold:0
  cmti:ComputerModern-Italic:0 
  cmbxti:ComputerModern-BoldItalic:0
  cmss:ComputerModernSans-Regular:0
  cmssbx:ComputerModernSans-Bold:0
  cmssi:ComputerModernSans-Italic:0
  cmssxi:ComputerModernSans-BoldItalic:0
  cmtt:ComputerModernTypewriter-Regular:1
  cmitt:ComputerModernTypewriter-Italic:1
)

SIZES=(8 9 10 12 14 17 24)
DEVICES=(inkplate6:166 inkplate10:150 inkplate6PLUS:212)

mag_step=0

for device in ${DEVICES[@]}; do

  dev_name=$(echo $device | cut -d ":" -f 1)
  dev_dpi=$(echo $device | cut -d ":" -f 2)

  for font in ${FONTS[@]}; do

    font_name=$(echo $font | cut -d ":" -f 1)
    ibmf_name=$(echo $font | cut -d ":" -f 2)
    char_set=$(echo $font | cut -d ":" -f 3)

    for size in ${SIZES[@]}; do
      mf '\smode="'${dev_name}'"; mag=magstep '${mag_step}'; input 'mf/${font_name}${size}''
      gftopk "${font_name}${size}.${dev_dpi}gf"
      rm "${font_name}${size}.${dev_dpi}gf"
      rm "${font_name}${size}.log"
    done

    ../.pio/build/generator/program "." ${ibmf_name} ${dev_dpi} ${char_set} ${font_name} ${SIZES[@]}
    rm *pk
    rm *tfm
  done
done

echo Done

