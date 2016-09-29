#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "nordic_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "app_trace.h"
#include "ble_db_discovery.h"
#include "app_timer.h"
#include "app_util.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "boards.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_hci.h"
#include "softdevice_handler.h"
#include "ble_advdata.h"
#include "ble_nus_c.h"
//#include "app_mailbox.h"
#include "nrf_drv_spi.h"
#include "user_ethernet.h"
#include "user_spi.h"

#include "loopback.h"
#include "w5500.h"
#include "socket.h"

#include "user_ble.h"
#include "user_uart.h"


#define APP_TIMER_PRESCALER     0                               /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE 2                               /**< Size of timer operation queues. */

#define BUFFER_SIZE					2048

APP_TIMER_DEF(tcp_con_timer_id);												/**< Publish data timer. */
#define TCP_CON_INTERVAL             	APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)	/**< RR interval interval (ticks). */

APP_TIMER_DEF(ble_to_tcps_timer_id);												/**< Publish data timer. */
#define BLE_TO_TCPS_INTERVAL             	APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)	/**< RR interval interval (ticks). */

APP_TIMER_DEF(tcps_to_ble_timer_id);												/**< Publish data timer. */
#define TCPS_TO_BLE_INTERVAL             	APP_TIMER_TICKS(100, APP_TIMER_PRESCALER)	/**< RR interval interval (ticks). */

#define SOCK_TCPC 1

volatile static uint8_t ble_input_data[BLE_NUS_MAX_DATA_LEN];
volatile static uint8_t ble_input_data_len;
volatile static bool data_in_flag = false;

extern user_m_ble_nus_t  user_m_ble_nus_c;                    /**< User NUS Service Class. */

uint8_t tempBuffer[BUFFER_SIZE];
uint8_t targetIP[4] = {192,168,1,33};
uint32_t tcp_targetPort = 5000;

/**@brief   Function for handling app_uart events.
 *
 * @details This function will receive a single character from the app_uart module and append it to 
 *          a string. The string will be be sent over BLE when the last character received was a 
 *          'new line' i.e '\n' (hex 0x0D) or if the string has reached a length of 
 *          @ref NUS_MAX_DATA_LENGTH.
 */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint8_t index = 0;

    switch (p_event->evt_type)
    {
        /**@snippet [Handling data from UART] */ 
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;

            if ((data_array[index - 1] == '\n') || (index >= (BLE_NUS_MAX_DATA_LEN)))
            {
                while (ble_nus_c_string_send(&user_m_ble_nus_c.m_ble_nus_c, data_array, index) != NRF_SUCCESS)
                {
                    // repeat until sent.
                }
                index = 0;
            }
            break;
        /**@snippet [Handling data from UART] */ 
        case APP_UART_COMMUNICATION_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}


/**@brief Callback handling NUS Client events.
 *
 * @details This function is called to notify the application of NUS client events.
 *
 * @param[in]   p_ble_nus_c   NUS Client Handle. This identifies the NUS client
 * @param[in]   p_ble_nus_evt Pointer to the NUS Client event.
 */

/**@snippet [Handling events from the ble_nus_c module] */ 
static void ble_nus_c_evt_handler(ble_nus_c_t * p_ble_nus_c, const ble_nus_c_evt_t * p_ble_nus_evt)
{
    uint32_t err_code, i;

    switch (p_ble_nus_evt->evt_type)
    {
        case BLE_NUS_C_EVT_DISCOVERY_COMPLETE:
            err_code = ble_nus_c_handles_assign(p_ble_nus_c, p_ble_nus_evt->conn_handle, &p_ble_nus_evt->handles);
            APP_ERROR_CHECK(err_code);

            err_code = ble_nus_c_rx_notif_enable(p_ble_nus_c);
            APP_ERROR_CHECK(err_code);
            APPL_LOG("The device has the Nordic UART Service\r\n");
            break;
        
        case BLE_NUS_C_EVT_NUS_RX_EVT:
						printf("[From BLE_P] ");
						printf("%02x:%02x:%02x:%02x:%02x:%02x ", user_m_ble_nus_c.peer_addr.addr[0], user_m_ble_nus_c.peer_addr.addr[1], 
				              user_m_ble_nus_c.peer_addr.addr[2], user_m_ble_nus_c.peer_addr.addr[3], user_m_ble_nus_c.peer_addr.addr[4], 
				              user_m_ble_nus_c.peer_addr.addr[5]);
            for ( i = 0; i < p_ble_nus_evt->data_len; i++)
            {
                while(app_uart_put( p_ble_nus_evt->p_data[i]) != NRF_SUCCESS);
            }
            ble_input_data_len = i;
            memcpy((void *)ble_input_data, p_ble_nus_evt->p_data, ble_input_data_len);//hoon
            data_in_flag = true;
            break;
        
        case BLE_NUS_C_EVT_DISCONNECTED:
            APPL_LOG("Disconnected\r\n");
            scan_start();
            break;
    }
}

/**@snippet [Handling events from the ble_nus_c module] */ 

/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    uint32_t err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from the BSP module.
 *
 * @param[in] event  Event generated by button press.
 */
void bsp_event_handler(bsp_event_t event)
{
    uint32_t err_code;
    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break;

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(user_m_ble_nus_c.m_ble_nus_c.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break;

        default:
            break;
    }
}

/**@brief Function for initializing buttons and leds.
 */
static void buttons_leds_init(void)
{
    bsp_event_t startup_event;

    uint32_t err_code = bsp_init(BSP_INIT_LED,
                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
                                 bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);
}

/** @brief Function for the Power manager.
 */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();

    APP_ERROR_CHECK(err_code);
}

void ble_to_tcps(void * p_context)
{
    int32_t ret;
		char buf[100] ;
	
    if (data_in_flag)
    {
        if (getSn_SR(SOCK_TCPC) == SOCK_ESTABLISHED)
        {
            sprintf(buf, "[From BLE_P] %02x:%02x:%02x:%02x:%02x:%02x ", 
					            user_m_ble_nus_c.peer_addr.addr[0], user_m_ble_nus_c.peer_addr.addr[1], 
				              user_m_ble_nus_c.peer_addr.addr[2], user_m_ble_nus_c.peer_addr.addr[3], 
					            user_m_ble_nus_c.peer_addr.addr[4], user_m_ble_nus_c.peer_addr.addr[5]);
						
						ret = send(SOCK_TCPC,  (void *)buf, strlen(buf));  // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
            ret = send(SOCK_TCPC,  (void *)ble_input_data, ble_input_data_len); 
            if(ret < 0) // Send Error occurred (sent data length < 0)
                close(SOCK_TCPC); // socket close
				}
			  data_in_flag = false;
    }
}


void tcps_to_ble(void * p_context)
{
	  uint32_t size, i, k=0;
    int32_t ret;
		char buf[DATA_BUF_SIZE];

    if((size = getSn_RX_RSR(SOCK_TCPC)) > 0) // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
    {
        size = MIN(size, DATA_BUF_SIZE); // DATA_BUF_SIZE means user defined buffer size (array)
        ret = recv(SOCK_TCPC, (uint8_t *)buf, size); // Data Receive process (H/W Rx socket buffer -> User's buffer)
			  if (ret<0)
				{
					  close(SOCK_TCPC); // socket close
				}
				else
				{
			      while(size)
				    {
					      i = MIN(size, BLE_NUS_MAX_DATA_LEN);
				        while (ble_nus_c_string_send(&user_m_ble_nus_c.m_ble_nus_c, (uint8_t *)(buf+k), i) != NRF_SUCCESS)
                {
                      // repeat until sent.
                }
						    size-=i;
						    k+=i;
				    }
        }
	  }
}

void tcp_con_timer(void * p_context)
{
    int32_t ret; // return value for SOCK_ERRORs

   // Port number for TCP client (will be increased)
    uint16_t any_port = 	50000;

   // Socket Status Transitions
   // Check the W5500 Socket n status register (Sn_SR, The 'Sn_SR' controlled by Sn_CR command or Packet send/recv status)
    switch(getSn_SR(SOCK_TCPC))
    {
        case SOCK_CLOSE_WAIT :
#ifdef _LOOPBACK_DEBUG_
           printf("%d:CloseWait\r\n",SOCK_TCPC);
#endif
            if((ret=disconnect(SOCK_TCPC)) != SOCK_OK) printf("%d:Socket Closed Fail\r\n", SOCK_TCPC);
#ifdef _LOOPBACK_DEBUG_
            printf("%d:Socket Closed\r\n", SOCK_TCPC);
#endif
            break;

        case SOCK_INIT :
#ifdef _LOOPBACK_DEBUG_
            printf("%d:Try to connect to the %d.%d.%d.%d : %d\r\n", 
				        SOCK_TCPC, targetIP[0], targetIP[1], targetIP[2], targetIP[3], tcp_targetPort);
#endif
            if( (ret = connect(SOCK_TCPC, targetIP, tcp_targetPort)) != SOCK_OK) 
						    printf("%d:Socket Connect Fail\r\n", SOCK_TCPC);	//	Try to TCP connect to the TCP server (destination)
            ret = send(SOCK_TCPC,  (void *)"connected\r\n", 11);
        break;

        case SOCK_CLOSED:
            close(SOCK_TCPC);
    	      if((ret=socket(SOCK_TCPC, Sn_MR_TCP, any_port++, 0x00)) != SOCK_TCPC) 
						    printf("%d:Socket Open Fail\r\n", SOCK_TCPC); // TCP socket open with 'any_port' port number
            break;
						
        default:
            break;
    }
}

static void user_app_timer_init(void)
{
    uint32_t err_code;
    
    err_code = app_timer_create(&tcp_con_timer_id, APP_TIMER_MODE_REPEATED, tcp_con_timer);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_create(&ble_to_tcps_timer_id, APP_TIMER_MODE_REPEATED, ble_to_tcps);
    APP_ERROR_CHECK(err_code);
	
    err_code = app_timer_create(&tcps_to_ble_timer_id, APP_TIMER_MODE_REPEATED, tcps_to_ble);
    APP_ERROR_CHECK(err_code);	
	
}

static void user_app_timer_start(void)
{
    uint32_t err_code;
    
    err_code = app_timer_start(tcp_con_timer_id, TCP_CON_INTERVAL,NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(ble_to_tcps_timer_id, BLE_TO_TCPS_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(tcps_to_ble_timer_id, TCPS_TO_BLE_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);	
}

int main(void)
{
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, NULL);

    uart_init(uart_event_handle, APP_IRQ_PRIORITY_LOW);
    buttons_leds_init();
    db_discovery_init();
    ble_stack_init();
    nus_c_init(ble_nus_c_evt_handler);

  	spi0_master_init();
    user_ethernet_init();

    user_app_timer_init();
    user_app_timer_start();

    // Start scanning for peripherals and initiate connection
    // with devices that advertise NUS UUID.
    scan_start();
    printf("Scan started\r\n");

    for (;;)
    {
        power_manage();
    }
}
