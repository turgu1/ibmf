#! /bin/bash

# FONTS=(ecti:tcti:EC-Italic:0)
FONTS=(
  ecrm:tcrm:EC-Regular:0
  ecss:tcss:ECSans-Regular:0
)
# FONTS=(
#   ecrm:tcrm:EC-Regular:0
#   ecbx:tcbx:EC-Bold:0
#   ecti:tcti:EC-Italic:0 
#   ecbi:tcbi:EC-BoldItalic:0
#   ecss:tcss:ECSans-Regular:0
#   ecsx:tcsx:ECSans-Bold:0
#   ecsi:tcsi:ECSans-Italic:0
#   ecso:tcso:ECSans-BoldItalic:0
#   ectt:tctt:ECTypewriter-Regular:0
#   ecit:tcit:ECTypewriter-Italic:0
# )

SIZES=(12:1200 14:1440)
IN_SIZES=(1200 1440)
OUT_SIZES=(12 14)
# SIZES=(10:1000)
# IN_SIZES=(1000)
# OUT_SIZES=(10)
# SIZES=(8:0800 9:0900 10:1000 12:1200 14:1440 17:1728 24:2488)
# IN_SIZES=(0800 0900 1000 1200 1440 1728 2488)
# OUT_SIZES=(8 9 10 12 14 17 24)

DEVICES=(inkplate6PLUS:212 solreader100:100 solreader75:75)
# DEVICES=(inkplate6:166 inkplate10:150 inkplate6PLUS:212)

mag_step=0

for device in ${DEVICES[@]}; do

  dev_name=$(echo $device | cut -d ":" -f 1)
  dev_dpi=$(echo $device | cut -d ":" -f 2)

  for font in ${FONTS[@]}; do

    font_name=$(echo $font | cut -d ":" -f 1)
    comp_name=$(echo $font | cut -d ":" -f 2)
    ibmf_name=$(echo $font | cut -d ":" -f 3)
    char_set=$(echo $font | cut -d ":" -f 4)

    for size in ${SIZES[@]}; do
    
      out_size=$(echo $size | cut -d ":" -f 1)
      in_size=$(echo $size | cut -d ":" -f 2)
      
      mf '\smode="'${dev_name}'"; mag=magstep '${mag_step}'; input 'ec/${font_name}${in_size}''
      gftopk "${font_name}${in_size}.${dev_dpi}gf"
      mf '\smode="'${dev_name}'"; mag=magstep '${mag_step}'; input 'ec/${comp_name}${in_size}''
      gftopk "${comp_name}${in_size}.${dev_dpi}gf"

      rm "${font_name}${in_size}.${dev_dpi}gf"
      rm "${font_name}${in_size}.log"
      rm "${comp_name}${in_size}.${dev_dpi}gf"
      rm "${comp_name}${in_size}.log"

      mv "${font_name}${in_size}.${dev_dpi}pk" "${font_name}${out_size}.${dev_dpi}pk"
      mv "${font_name}${in_size}.tfm" "${font_name}${out_size}.tfm"
      mv "${comp_name}${in_size}.${dev_dpi}pk" "${comp_name}${out_size}.${dev_dpi}pk"
      mv "${comp_name}${in_size}.tfm" "${comp_name}${out_size}.tfm"
      
    done

    ../.pio/build/generator_v3/program "." ${ibmf_name} ${dev_dpi} ${char_set} ${font_name} ${comp_name} ${OUT_SIZES[@]}
    rm *pk
    rm *tfm
  done
done

echo Done

