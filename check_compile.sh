#Everybody attention!
#This is important!
#Run this script before your commit, make sure all board config of amlogic can be compiled successfully.
#

declare -i RESULT
declare DETAIL
RESULT=0
make distclean
make help
make m6_skt_v1_config
make -j
if [ $? != 0 ]
then DETAIL=$DETAIL'#------------m6_skt_v1------failed-----#\n'
     RESULT=$RESULT+1
else DETAIL=$DETAIL'#------------m6_skt_v1------pass-------#\n'
fi

make distclean
make help
make m6_ref_v1_config
make -j
if [ $? != 0 ]
then DETAIL=$DETAIL'#------------m6_ref_v1------failed-----#\n'
     RESULT=$RESULT+1
else DETAIL=$DETAIL'#------------m6_ref_v1------pass-------#\n'
fi

make distclean
make help
make m6_ref_v2_config
make -j
if [ $? != 0 ]
then DETAIL=$DETAIL'#------------m6_ref_v2------failed-----#\n'
     RESULT=$RESULT+1
else DETAIL=$DETAIL'#------------m6_ref_v2------pass-------#\n'
fi

make distclean
make help
make m6l_skt_v1_config
make -j
if [ $? != 0 ]
then DETAIL=$DETAIL'#------------m6l_skt_v1-----failed-----#\n'
     RESULT=$RESULT+1
else DETAIL=$DETAIL'#------------m6l_skt_v1-----pass-------#\n'
fi

make distclean
make help
make m6s_skt_v1_config
make -j
if [ $? != 0 ]
then DETAIL=$DETAIL'#------------m6s_skt_v1-----failed-----#\n'
     RESULT=$RESULT+1
else DETAIL=$DETAIL'#------------m6s_skt_v1-----pass-------#\n'
fi

make distclean
make help
make m6tv_skt_v1_config
make -j
if [ $? != 0 ]
then DETAIL=$DETAIL"#------------m6tv_skt_v1----failed-----#\n"
     RESULT=$RESULT+1
else DETAIL=$DETAIL"#------------m6tv_skt_v1----pass-------#\n"
fi

make distclean
make help
make m6tv_ref_v1_config
make -j
if [ $? != 0 ]
then DETAIL=$DETAIL"#------------m6tv_ref_v1----failed-----#\n"
     RESULT=$RESULT+1
else DETAIL=$DETAIL"#------------m6tv_ref_v1----pass-------#\n"
fi


echo "#--------------------------------------#"
echo "#---------compile check result---------#"
echo "#--------------------------------------#"
if [ $RESULT = 0 ]
then
    echo "#-----------------PASS-----------------#"
else
    echo "#----------------FAILED----------------#"
fi
echo "#--------------------------------------#"
echo "#---------------DETAIL-----------------#"
echo -e $DETAIL
