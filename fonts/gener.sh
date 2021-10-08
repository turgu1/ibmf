#! /bin/bash


FONTS=(
  cmr:ComputerModern-Regular 
  cmbx:ComputerModern-Bold 
  cmti:ComputerModern-Italic 
  cmbxti:ComputerModern-Bold-Italic
  cmss:ComputerModernSans-Regular 
  cmssbx:ComputerModernSans-Bold 
  cmssi:ComputerModernSans-Italic 
  cmssxi:ComputerModernSans-Bold-Italic
  cmtt:ComputerModernTypewriter-Regular 
  cmitt:ComputerModernTypewriter-Italic
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

    for size in ${SIZES[@]}; do
      mf '\smode="'${dev_name}'"; mag=magstep '${mag_step}'; input 'mf/${font_name}${size}''
      gftopk "${font_name}${size}.${dev_dpi}gf"
      rm "${font_name}${size}.${dev_dpi}gf"
      rm "${font_name}${size}.log"
    done

    ../.pio/build/generator_release/program "." ${ibmf_name} ${dev_dpi} ${font_name} ${SIZES[@]}
    rm *pk
    rm *tfm
  done
done

echo Done

