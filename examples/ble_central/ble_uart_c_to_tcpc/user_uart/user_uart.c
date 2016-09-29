#include "user_uart.h"

/**@brief Function for initializing the UART.
 */
void uart_init(app_uart_event_handler_t event_handler, app_irq_priority_t irq_priority)
{
    uint32_t err_code;

    const app_uart_comm_params_t comm_params =
      {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = 0xFF,
        .cts_pin_no   = 0xFF,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
      };	

    APP_UART_FIFO_INIT(&comm_params,
                        UART_RX_BUF_SIZE,
                        UART_TX_BUF_SIZE,
                        event_handler,
                        irq_priority,
                        err_code);

    APP_ERROR_CHECK(err_code);
}


