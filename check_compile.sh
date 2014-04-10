#!/bin/bash

#------------IMPORTANT------------#
#--RUN THIS SCRIPT BEFOR COMMIT---#
#---------------------------------#

#usage:
#
#./check_compile.sh            -check amlogic board configs (ref: board/amlogic/boards.cfg)
#./check_compile.sh cus        -check customer board configs (ref: customer/board/boards.cfg)
#./check_compile.sh all        -check both amlogic and customer boards


declare -a cfg_file=[]

if [ "$1" == "all" ]
then
  if [ -f "customer/board/boards.cfg" ]
  then
    cfg_file[0]="board/amlogic/boards.cfg"
    cfg_file[1]="customer/board/boards.cfg"
  else
    #if customer folder doesn't exit
    cfg_file[0]="board/amlogic/boards.cfg"
  fi
else
  if [ "$1" == "cus" ]
  then
    cfg_file[0]="customer/board/boards.cfg"
  else
    cfg_file[0]="board/amlogic/boards.cfg"
  fi
fi

declare RESULT
declare -i LOOP_NUM
declare -i CFG_START=0

for cfg in ${cfg_file[@]}
do
  RESULT=$RESULT'\n#'
  for x in $(seq 20)
  do RESULT=$RESULT'~'
  done
  if [ "$cfg" == "board/amlogic/boards.cfg" ]
  then
    RESULT=$RESULT'AMLOGIC BOARDS'
  else
    RESULT=$RESULT'CUSTOMER BOARD'
  fi
  for x in $(seq 20)
  do RESULT=$RESULT'~'
  done
  RESULT=$RESULT'#\n'

  declare -a ARRAY
  declare -i TOTAL
  while read line
  do
    if [[ $CFG_START = 1 ]]
    #if get start position of configs
    then
      if [ -n "$line" ] && [ "#" != `expr substr "$line" 1 1` ]
      then
        #echo -n "one line: "
        #store each configs
        ind=`expr index "$line" ' '`
        sub=`expr substr "$line" 1 "$ind"`
        ARRAY[$TOTAL]=$sub
        TOTAL=$TOTAL+1
      else
        #blank line process
        if [ -z "$line" ]
        then
          continue
        else
          #re-meet # lines, means end of configs
          if [ "#" = `expr substr "$line" 1 1` ]
          then
            break
          fi
        fi
      fi
    else
      #try to get start position
      str_length=${#line}
      #echo $str_length
      if [ "$str_length" -gt "10" ]
      then
        cfg_str=`expr substr "$line" 1 10`
        #start position synbol: a line of #
        if [ "$cfg_str" == "##########" ]
        then
          CFG_START=1
        fi
      fi
    fi
  done < $cfg

  #run make and print result
  declare -i BAR_TOTAL=25
  declare -i BAR_LOOP
  for cfg in ${ARRAY[@]}
  do
    LOOP_NUM=$LOOP_NUM+1
    RESULT=$RESULT'#--------'
    if [ "$LOOP_NUM" -lt "10" ]
    then RESULT=$RESULT'00'$LOOP_NUM
    else
      if [ "$LOOP_NUM" -lt "100" ]
      then RESULT=$RESULT'0'$LOOP_NUM
      else RESULT=$RESULT$LOOP_NUM
      fi
    fi
    RESULT=$RESULT'--------'$cfg
    BAR_LOOP=BAR_TOTAL-`expr length $cfg`
    if [ "$BAR_LOOP" -gt "0" ]
    then
      for tmp in `seq $BAR_LOOP`;do RESULT=$RESULT'-';done
    fi
    make distclean
    make $cfg -j
    if [ $? != 0 ]
    then RESULT=$RESULT'-failed---'
    else RESULT=$RESULT'-pass-----'
    fi
    RESULT=$RESULT'#\n'
  done
  CFG_START=0
  unset ARRAY
  TOTAL=0
done

echo -e $RESULT
