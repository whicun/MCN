/*
 * can.c
 *
 *  Created on: Nov 12, 2013
 *      Author: Nathan
 */
#include "../MCN1 Headers/all.h"

struct ECAN_REGS ECanaShadow;

/*
 * Function responsible for initializing the CAN module.
 *
 * For simple CAN setup the function CreateCANMailbox can be used to specify user
 * mailboxes.
 */
void CANSetup()
{
	//System specific CAN setup
	SystemCANInit(&ECanaShadow);

	EALLOW;
	//MBOX 0 - 1
	CommandBoxInit(COMMAND_ID);   // Mbox 0
	HeartbeatBoxInit(HEARTBEAT_ID); // Mbox 1
	/*
	* todo USER: Node specifc CAN setup
	* create mailbox for all Receive and transmit IDs
	* MBOX2 - MBOX31
    *
	* CreateCANMailbox(int mailboxNum, int IDE, int AME, int AAM, int DLC, int STDMSGID, int Mode)
	*/

	//adc TRANSMIT
	//CreateCANMailbox(2,0,0,1,8,ADC_ID,0);
	//gp_button TRANSMIT
	//CreateCANMailbox(3,0,0,1,8,GP_BUTTON_ID,0);

	CreateCANMailbox(SUSPENSION_TRAVELS_BOX,0,0,1,4,SUSPENSION_TRAVELS_ID,0); //CHECK AAM
	CreateCANMailbox(STEERING_BOX,0,0,1,4,STEERING_ID,0);
	CreateCANMailbox(AMBIENT_MEASUREMENTS_BOX,0,0,1,8,AMBIENT_MEASUREMENTS_ID, 0);
	CreateCANMailbox(COOLANT_PRESSURES_BOX,0,0,1,8,COOLANT_PRESSURES_ID,0);

	ECanaShadow.CANMD.bit.MD9 = 1;			//receive
	ECanaShadow.CANME.bit.ME9 = 1;			//enable
	ECanaShadow.CANMIM.bit.MIM9  = 1; 		//int enable
	ECanaShadow.CANMIL.bit.MIL9  = 1;  		// Int.-Level MB#0  -> I1EN

    EDIS;
    FinishCANInit();
}

char FillCAN(unsigned int Mbox)
{
	CopyMCToShadow(&ECanaShadow);

	//Try to fill heartbeat. If not heartbeat mailbox, see if it's a user mailbox
	if(FillHeartbeat(Mbox, user_ops.UserFlags.all) != 1)
	{
		//todo USER: setup for all transmit MBOXs
		//InsertCANMessage(int Mbox, unsigned int MDH, unsigned int MDL)
		switch (Mbox)
		{
		case SUSPENSION_TRAVELS_BOX:
			InsertCANMessage(SUSPENSION_TRAVELS_BOX, user_data.rear_suspension_travel.U32, user_data.front_suspension_travel.U32);
			return 1;
		case STEERING_BOX:
			InsertCANMessage(STEERING_BOX, 0, user_data.steering_angle.U32);
			return 1;
		case AMBIENT_MEASUREMENTS_BOX:
			InsertCANMessage(AMBIENT_MEASUREMENTS_BOX, user_data.ambient_pressure.U32, user_data.ambient_temperature.U32);
			return 1;
		case COOLANT_PRESSURES_BOX:
			InsertCANMessage(COOLANT_PRESSURES_BOX, 0, user_data.motor_inlet_pressure.U32);
			return 1;

		case COOLANT_FLOW_BOX:
			InsertCANMessage(COOLANT_FLOW_BOX, 0, user_data.coolant_flow.U32);
			return 1;
		case POWERTRAIN_COOLANT_TEMP_BOX:
			InsertCANMessage(POWERTRAIN_COOLANT_TEMP_BOX, user_data.motor_control_coolant_temp.U32, user_data.motor_coolant_temp.U32);
			return 1;
		case MOTOR_AIR_PRESSURES_BOX:
			InsertCANMessage(MOTOR_AIR_PRESSURES_BOX, user_data.motor_air_pressure_2.U32, user_data.motor_air_pressure_1.U32);
			return 1;
		case MOTOR_PLATE_TEMPS_BOX:
			InsertCANMessage(MOTOR_PLATE_TEMPS_BOX, user_data.motor_plate_temp_2.U32, user_data.motor_plate_temp_1.U32);
			return 1;
		case STRAIN_GAUGE_12_BOX:
			InsertCANMessage(STRAIN_GAUGE_12_BOX, user_data.strain_gauge_2.U32, user_data.strain_gauge_1.U32);
			return 1;
		case STRAIN_GAUGE_34_BOX:
			InsertCANMessage(STRAIN_GAUGE_34_BOX, user_data.strain_gauge_4.U32, user_data.strain_gauge_3.U32);
			return 1;
		case STRAIN_GAUGE_56_BOX:
			InsertCANMessage(STRAIN_GAUGE_56_BOX, user_data.strain_gauge_6.U32, user_data.strain_gauge_5.U32);
			return 1;
		case TRITIUM_RESET_BOX:
			InsertCANMessage(TRITIUM_RESET_BOX, 0, 0);
		case SUPPLY_BOX:
			InsertCANMessage(SUPPLY_BOX, 0, user_data.v12.U32);
		default:
			return 0;
		}
	}
	else
	{
		return 1;
	}
}

void FillCANData()
{
	//todo USER: use FillCAN to put data into correct mailboxes
	FillCAN(COOLANT_FLOW_BOX);
	FillCAN(POWERTRAIN_COOLANT_TEMP_BOX);
	FillCAN(MOTOR_AIR_PRESSURES_BOX);
	FillCAN(MOTOR_PLATE_TEMPS_BOX);
	FillCAN(STRAIN_GAUGE_12_BOX);
	FillCAN(STRAIN_GAUGE_34_BOX);
	FillCAN(STRAIN_GAUGE_56_BOX);
	FillCAN(SUPPLY_BOX);

	FillCAN(SUSPENSION_TRAVELS_BOX);
	FillCAN(STEERING_BOX);
	FillCAN(AMBIENT_MEASUREMENTS_BOX);
	FillCAN(COOLANT_PRESSURES_BOX);
}

// INT9.6
__interrupt void ECAN1INTA_ISR(void)  // eCAN-A
{
	Uint32 ops_id;
	Uint32 dummy;
	Uint16 errors;
  	unsigned int mailbox_nr;
  	mailbox_nr = getMailboxNR();
  	//todo USER: Setup ops command
  	if(mailbox_nr == COMMAND_BOX)
  	{
  		ReadCommand();
  	}
  	/*
  	else if(mailbox_nr == TRITIUM_ERRORS_BOX)
  	{
  		errors = ECanaMboxes.MBOX9.MDL.byte.BYTE1;
  		if(errors > 0)
  		{
  			SendCAN(TRITIUM_RESET_BOX);
  		}
		ECanaRegs.CANRMP.bit.RMP9 = 1;

  	}
  	*/
  	//todo USER: Setup other reads



  	//To receive more interrupts from this PIE group, acknowledge this interrupt
  	PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}

// INT9.5
__interrupt void ECAN0INTA_ISR(void)   // eCAN-A
{

	// To receive more interrupts from this PIE group, acknowledge this interrupt
	PieCtrlRegs.PIEACK.all = PIEACK_GROUP9;
}
