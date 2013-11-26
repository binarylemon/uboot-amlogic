help: help_customer

	@echo =======================================================================
	@echo The mark in board is "Mesonboard_AML8726-M_V1.1"
	@echo config command: \"make arm_8726m_config\"

	@echo =======================================================================
	@echo The mark in board is "M3_SKT_V1"
	@echo config command: \"make m3_skt_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "WA_M3_REF_V1"
	@echo config command: \"make m3_wa_ref_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M6_SKT_V1"
	@echo config command: \"make m6_skt_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M6_REF_V1"
	@echo config command: \"make m6_ref_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M6_REF_V2"
	@echo config command: \"make m6_ref_v2_config\"

	@echo =======================================================================
	@echo The mark in board is "M6S_SKT_V1", relative to chip M6S
	@echo config command: \"make m6s_skt_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M6L_SKT_V1", relative to chip M6l
	@echo config command: \"make m6l_skt_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M6TV_SKT_V1"
	@echo config command: \"make m6tv_skt_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M6TV_REF_V1"
	@echo config command: \"make m6tv_ref_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M8_PXP"
	@echo config command: \"make m8_pxp_config\"
	
	@echo =======================================================================
	@echo The mark in board is "M8_ZEBU"
	@echo config command: \"make m8_ZeBu_config\"
	
	@echo =======================================================================
	@echo The mark in board is "M8_SKT_V1"
	@echo config command: \"make m8_skt_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M8_K200_V1"
	@echo config command: \"make m8_k200_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M8_K100_V1"
	@echo config command: \"make m8_k100_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "M8_K04_M3X13_V1"
	@echo config command: \"make m8_k04_m3x13_v1_config\"


	@echo =======================================================================
	@echo The mark in board is "M8_K101_V1"
	@echo config command: \"make m8_k101_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "m8_k03_M101_v1"
	@echo config command: \"make m8_k03_M101_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "m8_k03_M102_v1"
	@echo config command: \"make m8_k03_M102_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "m8_k03_M901_v1"
	@echo config command: \"make m8_k03_M901_v1_config\"

	@echo =======================================================================
	@echo The mark in board is "m8_k03_M902_v1"
	@echo config command: \"make m8_k03_M902_v1_config\"
	
	@echo =======================================================================
	@echo The mark in board is "m8_k05_hp"
	@echo config command: \"make m8_k05_hp_config\"
	
	@echo =======================================================================
	@echo The mark in board is "m8_k06_NabiJR_v1"
	@echo config command: \"make m8_k06_NabiJR_v1_config\"
	
	@echo

sinclude $(wildcard $(SRCTREE)/customer/board/Readme.mk)
