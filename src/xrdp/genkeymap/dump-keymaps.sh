#!/bin/sh

which setxkbmap
if test $? -ne 0
then
  echo "error, setxkbmap not found"
  exit 1
fi

# Finish - FI 'fi' 0x040b
echo "Finish"
setxkbmap -model pc104 -layout fi
./xrdp-genkeymap ../instfiles/km-040b.ini

# Arabic - AR 'ar' 0x0401
echo "Arabic"
setxkbmap -model pc104 -layout ar
./xrdp-genkeymap ../instfiles/km-0401.ini

# United Kingdom - GB 'en-gb' 0x0809
echo "United Kingdom"
setxkbmap -model pc104 -layout gb
./xrdp-genkeymap ../instfiles/km-0809.ini

# Belgium - FR 'fr-be' 0x080c
echo "Belgium"
setxkbmap -model pc104 -layout be
./xrdp-genkeymap ../instfiles/km-080c.ini

# Lithuanian - LT 'lt' 0x0427
echo "Lithuanian"
setxkbmap -model pc104 -layout lt
./xrdp-genkeymap ../instfiles/km-0427.ini

# Norwegian - NO 'no' 0x0414
echo "Norwegian"
setxkbmap -model pc104 -layout no
./xrdp-genkeymap ../instfiles/km-0414.ini

# Portuguese - PT 'pt-br' 0x0416
echo "Portuguese"
setxkbmap -model pc104 -layout br
./xrdp-genkeymap ../instfiles/km-0416.ini

# Danish - DA 'dk' 0x0406
echo "Danish"
setxkbmap -model pc104 -layout dk
./xrdp-genkeymap ../instfiles/km-0406.ini

# French - FR 'fr' 0x040c
echo "French"
setxkbmap -model pc104 -layout fr
./xrdp-genkeymap ../instfiles/km-040c.ini

# Japanese - JA 'jp' 0x0411
echo "Japanese"
setxkbmap -model pc104 -layout jp
./xrdp-genkeymap ../instfiles/km-0411.ini

# Latvian - LV 'lv' 0x0426
echo "Latvian"
setxkbmap -model pc104 -layout lv
./xrdp-genkeymap ../instfiles/km-0426.ini

# Polish - PL 'pl' 0x0415
echo "Polish"
setxkbmap -model pc104 -layout pl
./xrdp-genkeymap ../instfiles/km-0415.ini

# Turkish - TR 'tr' 0x041f
echo "Turkish"
setxkbmap -model pc104 -layout tr
./xrdp-genkeymap ../instfiles/km-041f.ini

# Spain - ES 'es' 0x040a
echo "Spain"
setxkbmap -model pc104 -layout es
./xrdp-genkeymap ../instfiles/km-040a.ini

# Croatia - HR 'hr' 0x041a
echo "Croatia"
setxkbmap -model pc104 -layout hr
./xrdp-genkeymap ../instfiles/km-041a.ini

# Macedonian - MK 'hr' 0x042f
echo "Macedonian"
setxkbmap -model pc104 -layout mk
./xrdp-genkeymap ../instfiles/km-042f.ini

# Portugal - PT 'pt' 0x0816
echo "Portugal"
setxkbmap -model pc104 -layout pt
./xrdp-genkeymap ../instfiles/km-0816.ini

# Slovenian - SL 'si' 0x0424
echo "Slovenian"
setxkbmap -model pc104 -layout si
./xrdp-genkeymap ../instfiles/km-0424.ini

# English - US 'en-us' 0x0409
echo "English"
setxkbmap -model pc104 -layout us
./xrdp-genkeymap ../instfiles/km-0409.ini

# German 'de' 0x0407
echo "German"
setxkbmap -model pc104 -layout de
./xrdp-genkeymap ../instfiles/km-0407.ini

# Italy 'it' 0x0410
echo "Italy"
setxkbmap -model pc104 -layout it
./xrdp-genkeymap ../instfiles/km-0410.ini

# Russia 'ru' 0x0419
echo "Russia"
setxkbmap -model pc104 -layout ru
./xrdp-genkeymap ../instfiles/km-0419.ini

# Sweden 'se' 0x041d
echo "Sweden"
setxkbmap -model pc104 -layout se
./xrdp-genkeymap ../instfiles/km-041d.ini

# Sweden 'hu' 0x040e
echo "Hungary"
setxkbmap -model pc104 -layout hu
./xrdp-genkeymap ../instfiles/km-040e.ini

# set back to en-us
setxkbmap -model pc104 -layout us
