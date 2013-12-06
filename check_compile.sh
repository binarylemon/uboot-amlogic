#Everybody attention!
#This is important!
#Run this script before your commit, make sure all board config of amlogic can be compiled successfully.
#

#how to use
#./check_m6.sh (all)
#compile all configs or basic configs

declare -i RESULT=0
declare DETAIL
declare CFG
if [ "$1" == "all" ]
then
  declare -a CONFIG=(
    m6_skt_v1
    m6_ref_v1
    m6_ref_v2
    m6l_skt_v1
    m6s_skt_v1
    m6tv_skt_v1
    m6tv_ref_v1
    m6_ramos_v1
    m6_ainol_v1
    m6_ainol_e3
    m6_ainol_e4
    m6_ainol_848_v1
    m6_emdoor_1024_600
    m6_emdoor_1024_768
    m6_newsmy_v1
    m6_asd_pi3100
    m6_asd_pi3900
    m6_chinach_v1
    m6_g33new_512M
    m6_g33new_1GB
    m6_g33_1212
    m6_winaccord_v1
    m6_yifang_m7000nbd
    mx_dongle_g02
    m6_mbx_v1
    m6_mbx_v1_r2
    m6_dongle
    m6_dongle_v131
    m6_dongle_g40ref
    m6_duokan_mbx_g19ref
    m6tv_h02_v1
    m6tv_h03_v1
    m6_dvb_g17
    m6tv_h04_v1
    m6tv_h05_v1
    m6tv_h06_v1
    m6_gadmei_g11t80
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
    m8_k12_MA975M8_v1
    )
else
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
fi

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
  make -j
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
