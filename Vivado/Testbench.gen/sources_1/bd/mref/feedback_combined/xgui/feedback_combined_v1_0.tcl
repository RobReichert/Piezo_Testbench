# Definitional proc to organize widgets for parameters.
proc init_gui { IPINST } {
  ipgui::add_param $IPINST -name "Component_Name"
  #Adding Page
  set Page_0 [ipgui::add_page $IPINST -name "Page 0"]
  ipgui::add_param $IPINST -name "ADC_DATA_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "AXIS_TDATA_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "CFG_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "CONTINUOUS_OUTPUT" -parent ${Page_0}
  ipgui::add_param $IPINST -name "DAC_DATA_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "DDS_OUT_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_A_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_B_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_C_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_D_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_E_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_F_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_G_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_H_OFFSET" -parent ${Page_0}
  ipgui::add_param $IPINST -name "PARAM_WIDTH" -parent ${Page_0}
  ipgui::add_param $IPINST -name "SELECT_WIDTH" -parent ${Page_0}


}

proc update_PARAM_VALUE.ADC_DATA_WIDTH { PARAM_VALUE.ADC_DATA_WIDTH } {
	# Procedure called to update ADC_DATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.ADC_DATA_WIDTH { PARAM_VALUE.ADC_DATA_WIDTH } {
	# Procedure called to validate ADC_DATA_WIDTH
	return true
}

proc update_PARAM_VALUE.AXIS_TDATA_WIDTH { PARAM_VALUE.AXIS_TDATA_WIDTH } {
	# Procedure called to update AXIS_TDATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.AXIS_TDATA_WIDTH { PARAM_VALUE.AXIS_TDATA_WIDTH } {
	# Procedure called to validate AXIS_TDATA_WIDTH
	return true
}

proc update_PARAM_VALUE.CFG_WIDTH { PARAM_VALUE.CFG_WIDTH } {
	# Procedure called to update CFG_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.CFG_WIDTH { PARAM_VALUE.CFG_WIDTH } {
	# Procedure called to validate CFG_WIDTH
	return true
}

proc update_PARAM_VALUE.CONTINUOUS_OUTPUT { PARAM_VALUE.CONTINUOUS_OUTPUT } {
	# Procedure called to update CONTINUOUS_OUTPUT when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.CONTINUOUS_OUTPUT { PARAM_VALUE.CONTINUOUS_OUTPUT } {
	# Procedure called to validate CONTINUOUS_OUTPUT
	return true
}

proc update_PARAM_VALUE.DAC_DATA_WIDTH { PARAM_VALUE.DAC_DATA_WIDTH } {
	# Procedure called to update DAC_DATA_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.DAC_DATA_WIDTH { PARAM_VALUE.DAC_DATA_WIDTH } {
	# Procedure called to validate DAC_DATA_WIDTH
	return true
}

proc update_PARAM_VALUE.DDS_OUT_WIDTH { PARAM_VALUE.DDS_OUT_WIDTH } {
	# Procedure called to update DDS_OUT_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.DDS_OUT_WIDTH { PARAM_VALUE.DDS_OUT_WIDTH } {
	# Procedure called to validate DDS_OUT_WIDTH
	return true
}

proc update_PARAM_VALUE.PARAM_A_OFFSET { PARAM_VALUE.PARAM_A_OFFSET } {
	# Procedure called to update PARAM_A_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_A_OFFSET { PARAM_VALUE.PARAM_A_OFFSET } {
	# Procedure called to validate PARAM_A_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_B_OFFSET { PARAM_VALUE.PARAM_B_OFFSET } {
	# Procedure called to update PARAM_B_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_B_OFFSET { PARAM_VALUE.PARAM_B_OFFSET } {
	# Procedure called to validate PARAM_B_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_C_OFFSET { PARAM_VALUE.PARAM_C_OFFSET } {
	# Procedure called to update PARAM_C_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_C_OFFSET { PARAM_VALUE.PARAM_C_OFFSET } {
	# Procedure called to validate PARAM_C_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_D_OFFSET { PARAM_VALUE.PARAM_D_OFFSET } {
	# Procedure called to update PARAM_D_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_D_OFFSET { PARAM_VALUE.PARAM_D_OFFSET } {
	# Procedure called to validate PARAM_D_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_E_OFFSET { PARAM_VALUE.PARAM_E_OFFSET } {
	# Procedure called to update PARAM_E_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_E_OFFSET { PARAM_VALUE.PARAM_E_OFFSET } {
	# Procedure called to validate PARAM_E_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_F_OFFSET { PARAM_VALUE.PARAM_F_OFFSET } {
	# Procedure called to update PARAM_F_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_F_OFFSET { PARAM_VALUE.PARAM_F_OFFSET } {
	# Procedure called to validate PARAM_F_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_G_OFFSET { PARAM_VALUE.PARAM_G_OFFSET } {
	# Procedure called to update PARAM_G_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_G_OFFSET { PARAM_VALUE.PARAM_G_OFFSET } {
	# Procedure called to validate PARAM_G_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_H_OFFSET { PARAM_VALUE.PARAM_H_OFFSET } {
	# Procedure called to update PARAM_H_OFFSET when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_H_OFFSET { PARAM_VALUE.PARAM_H_OFFSET } {
	# Procedure called to validate PARAM_H_OFFSET
	return true
}

proc update_PARAM_VALUE.PARAM_WIDTH { PARAM_VALUE.PARAM_WIDTH } {
	# Procedure called to update PARAM_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.PARAM_WIDTH { PARAM_VALUE.PARAM_WIDTH } {
	# Procedure called to validate PARAM_WIDTH
	return true
}

proc update_PARAM_VALUE.SELECT_WIDTH { PARAM_VALUE.SELECT_WIDTH } {
	# Procedure called to update SELECT_WIDTH when any of the dependent parameters in the arguments change
}

proc validate_PARAM_VALUE.SELECT_WIDTH { PARAM_VALUE.SELECT_WIDTH } {
	# Procedure called to validate SELECT_WIDTH
	return true
}


proc update_MODELPARAM_VALUE.ADC_DATA_WIDTH { MODELPARAM_VALUE.ADC_DATA_WIDTH PARAM_VALUE.ADC_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.ADC_DATA_WIDTH}] ${MODELPARAM_VALUE.ADC_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.DDS_OUT_WIDTH { MODELPARAM_VALUE.DDS_OUT_WIDTH PARAM_VALUE.DDS_OUT_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.DDS_OUT_WIDTH}] ${MODELPARAM_VALUE.DDS_OUT_WIDTH}
}

proc update_MODELPARAM_VALUE.PARAM_WIDTH { MODELPARAM_VALUE.PARAM_WIDTH PARAM_VALUE.PARAM_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_WIDTH}] ${MODELPARAM_VALUE.PARAM_WIDTH}
}

proc update_MODELPARAM_VALUE.PARAM_A_OFFSET { MODELPARAM_VALUE.PARAM_A_OFFSET PARAM_VALUE.PARAM_A_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_A_OFFSET}] ${MODELPARAM_VALUE.PARAM_A_OFFSET}
}

proc update_MODELPARAM_VALUE.PARAM_B_OFFSET { MODELPARAM_VALUE.PARAM_B_OFFSET PARAM_VALUE.PARAM_B_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_B_OFFSET}] ${MODELPARAM_VALUE.PARAM_B_OFFSET}
}

proc update_MODELPARAM_VALUE.PARAM_C_OFFSET { MODELPARAM_VALUE.PARAM_C_OFFSET PARAM_VALUE.PARAM_C_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_C_OFFSET}] ${MODELPARAM_VALUE.PARAM_C_OFFSET}
}

proc update_MODELPARAM_VALUE.PARAM_D_OFFSET { MODELPARAM_VALUE.PARAM_D_OFFSET PARAM_VALUE.PARAM_D_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_D_OFFSET}] ${MODELPARAM_VALUE.PARAM_D_OFFSET}
}

proc update_MODELPARAM_VALUE.PARAM_E_OFFSET { MODELPARAM_VALUE.PARAM_E_OFFSET PARAM_VALUE.PARAM_E_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_E_OFFSET}] ${MODELPARAM_VALUE.PARAM_E_OFFSET}
}

proc update_MODELPARAM_VALUE.PARAM_F_OFFSET { MODELPARAM_VALUE.PARAM_F_OFFSET PARAM_VALUE.PARAM_F_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_F_OFFSET}] ${MODELPARAM_VALUE.PARAM_F_OFFSET}
}

proc update_MODELPARAM_VALUE.PARAM_G_OFFSET { MODELPARAM_VALUE.PARAM_G_OFFSET PARAM_VALUE.PARAM_G_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_G_OFFSET}] ${MODELPARAM_VALUE.PARAM_G_OFFSET}
}

proc update_MODELPARAM_VALUE.PARAM_H_OFFSET { MODELPARAM_VALUE.PARAM_H_OFFSET PARAM_VALUE.PARAM_H_OFFSET } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.PARAM_H_OFFSET}] ${MODELPARAM_VALUE.PARAM_H_OFFSET}
}

proc update_MODELPARAM_VALUE.DAC_DATA_WIDTH { MODELPARAM_VALUE.DAC_DATA_WIDTH PARAM_VALUE.DAC_DATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.DAC_DATA_WIDTH}] ${MODELPARAM_VALUE.DAC_DATA_WIDTH}
}

proc update_MODELPARAM_VALUE.CFG_WIDTH { MODELPARAM_VALUE.CFG_WIDTH PARAM_VALUE.CFG_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.CFG_WIDTH}] ${MODELPARAM_VALUE.CFG_WIDTH}
}

proc update_MODELPARAM_VALUE.AXIS_TDATA_WIDTH { MODELPARAM_VALUE.AXIS_TDATA_WIDTH PARAM_VALUE.AXIS_TDATA_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.AXIS_TDATA_WIDTH}] ${MODELPARAM_VALUE.AXIS_TDATA_WIDTH}
}

proc update_MODELPARAM_VALUE.SELECT_WIDTH { MODELPARAM_VALUE.SELECT_WIDTH PARAM_VALUE.SELECT_WIDTH } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.SELECT_WIDTH}] ${MODELPARAM_VALUE.SELECT_WIDTH}
}

proc update_MODELPARAM_VALUE.CONTINUOUS_OUTPUT { MODELPARAM_VALUE.CONTINUOUS_OUTPUT PARAM_VALUE.CONTINUOUS_OUTPUT } {
	# Procedure called to set VHDL generic/Verilog parameter value(s) based on TCL parameter value
	set_property value [get_property value ${PARAM_VALUE.CONTINUOUS_OUTPUT}] ${MODELPARAM_VALUE.CONTINUOUS_OUTPUT}
}

