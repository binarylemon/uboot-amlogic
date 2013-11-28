#Everybody attention!
#This is important!
#Run this script before your commit, make sure all board config of amlogic can be compiled successfully.
#

declare -i RESULT=0
declare DETAIL
declare CFG
declare -a CONFIG=(
  m6_skt_v1
  m6_ref_v1
  m6_ref_v2
  m6l_skt_v1
  m6s_skt_v1
  m6tv_skt_v1
  m6tv_ref_v1
  m8_skt_v1
  m8_k200_v1
  m8_k100_v1
  m8_k101_v1
)

declare -i BAR_TOTAL=14
declare -i BAR_LOOP

for BD in ${CONFIG[@]}
do
  CFG=`echo "${BD}_config"`
  DETAIL=$DETAIL'#------------'
  DETAIL=$DETAIL$BD
  BAR_LOOP=BAR_TOTAL-`expr length $BD`
  if [ "$BAR_LOOP" -gt "0" ]
  then
    for tmp in `seq $BAR_LOOP`;do DETAIL=$DETAIL'-';done
  fi
  make distclean
  make $CFG
  make -j
  if [ $? != 0 ]
  then DETAIL=$DETAIL'---failed---'
    RESULT=$RESULT+1
  else DETAIL=$DETAIL'---pass-----'
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
