#! /bin/bash


FONTS=(cmr8:0 cmr9:0 cmr10:0 cmr12:0 cmr17:0)
DEVICES=(inkplate6:166 inkplate10:150 inkplate6PLUS:212)

for device in ${DEVICES[@]}; do
  dev_name=$(echo $device | cut -d ":" -f 1)
  dev_dpi=$(echo $device | cut -d ":" -f 2)
  for font in ${FONTS[@]}; do
    font_name=$(echo $font | cut -d ":" -f 1)
    mag_step=$(echo $font | cut -d ":" -f 2)
    mf '\smode="'${dev_name}'"; mag=magstep '${mag_step}'; input '${font_name}''
    gftopk "${font_name}.${dev_dpi}gf"
    rm "${font_name}.${dev_dpi}gf"
    rm "${font_name}.log"
    rm "${font_name}.tfm"
  done
done

echo Done

