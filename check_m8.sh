#Everybody attention!
#This is important!
#Run this script before your commit, make sure all board config of amlogic can be compiled successfully.
#

#how to use
#./check_m8.sh (all)
#compile all configs or basic configs

declare -i RESULT=0
declare DETAIL
declare CFG
if [ "$1" == "all" ]
then
  declare -a CONFIG=(
    m8_skt_v1
    m8_k200_v1
    m8_k100_v1
    m8_k101_v1
    m8_k01
    m8_k03_M101_v1
    m8_k03_M102_v1
    m8_k03_M901_v1
    m8_k03_M902_v1
    m8_k04_m3x13_v1
    m8_k05_hp
    m8_k06_NabiJR_v1
  )
else
  declare -a CONFIG=(
    m8_skt_v1
    m8_k200_v1
    m8_k100_v1
    m8_k101_v1
  )

declare -i BAR_TOTAL=20
declare -i BAR_LOOP

for BD in ${CONFIG[@]}
do
  CFG=`echo "${BD}_config"`
  DETAIL=$DETAIL'#--------'
  DETAIL=$DETAIL$BD
  BAR_LOOP=BAR_TOTAL-`expr length $BD`
  if [ "$BAR_LOOP" -gt "0" ]
  then
    for tmp in `seq $BAR_LOOP`;do DETAIL=$DETAIL'-';done
  fi
  make distclean
  make $CFG
  make
  if [ $? != 0 ]
  then DETAIL=$DETAIL'-failed---'
    RESULT=$RESULT+1
  else DETAIL=$DETAIL'-pass-----'
  fi
  DETAIL=$DETAIL'#\n'
done

echo "#--------------------------------------#"
echo "#---------compile check result---------#"
echo "#--------------------------------------#"
if [[ $RESULT = 0 ]]
then
    echo "#-----------------PASS-----------------#"
else
    echo "#----------------FAILED----------------#"
fi
echo "#--------------------------------------#"
echo "#---------------DETAIL-----------------#"
echo -e $DETAIL
