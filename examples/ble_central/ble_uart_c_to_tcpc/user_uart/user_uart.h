
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "app_error.h"
#include "app_uart.h"
#include "app_trace.h"
#include "boards.h"

#define UART_TX_BUF_SIZE        256                             /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE        256                             /**< UART RX buffer size. */

#define APPL_LOG                app_trace_log                   /**< Debug logger macro that will be used in this file to do logging of debug information over UART. */

void uart_init(app_uart_event_handler_t event_handler, app_irq_priority_t irq_priority);
