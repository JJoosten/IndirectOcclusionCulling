URL=http://5.39.89.158/ciView.php?id=cfw
FILE=current.response
FILELAST=last.response

while [[ 1 ]]
do
   wget -qO - $URL > $FILE
   if ! cmp $FILE $FILELAST > /dev/null 2>&1
   then
      cp -f $FILE $FILELAST
      echo Modification detected.

      #update git
      pushd .
      cd ../../..
      git pull
      popd

      #run mac build
      pushd .
      cd ../..
      ./build_mac.sh
      ./compile_mac.sh
      killall CrossFrameworkLauncher
      ./start_mac.sh &
      popd

      #run ios build
      pushd .
      cd ../..
      ./build_ios.sh
      ./compile_ios.sh
      ./start_ios_nodebug.sh &
      popd
   fi
   sleep 0.5
done
