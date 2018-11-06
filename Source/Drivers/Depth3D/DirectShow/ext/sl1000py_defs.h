//----------------------------------------------------------------------------
// SL1000 DVP UVC bypass
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Register Address/Reset Value
//----------------------------------------------------------------------------

#define  SYSVER0_ID     (0<<2)   //version reg0
#define  SYSVER1_ID     (1<<2)   //version reg1
#define  SYSCTRL_ID     (2<<2)   //control
#define  SYSCTRL0       ((2<<2)+0)   //control
#define  SYSCTRL1       ((2<<2)+1)   //control
#define  SYSCTRL2       ((2<<2)+2)   //control
#define  SYSCTRL3       ((2<<2)+3)   //control
#define  SYSSTA_ID      (3<<2)   //status
#define  SYSIN0W_ID     (4<<2)   //DVP IN0  The number of pixel clock periods in one line (row) time include visible and blankling
#define  SYSIN0H_ID     (5<<2)   //DVP IN0  The number of complete lines (rows) in the frame timing. include visible and blankling.
#define  SYSIN1W_ID     (6<<2)   //DVP IN1  The number of pixel clock periods in one line (row) time include visible and blankling
#define  SYSIN1H_ID     (7<<2)   //DVP IN1  The number of complete lines (rows) in the frame timing. include visible and blankling.
#define  PATCONF_ID     (8<<2)   
#define  TIGCONF_ID		(9<<2)
#define  DPUOLSTA_ID	(10<<2)   //status