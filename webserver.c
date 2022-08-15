#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "hardware/watchdog.h"
#include "hardware/structs/watchdog.h"

#include "tusb_lwip_glue.h"

#define LED_PIN     25
#define IN_PIN       1

const char * ssi_tags[] = {
  "GPIO",
  "COUNTER"
};

// let our webserver do some dynamic handling
static const char *cgi_toggle_led(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
    gpio_put(LED_PIN, !gpio_get(LED_PIN));
    return "/index.html";
}

static const char *cgi_reset_usb_boot(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]){
    reset_usb_boot(0, 0);
    return "/index.html";
}

static const tCGI cgi_handlers[] = {
  {
    "/toggle_led",
    cgi_toggle_led
  },
  {
    "/reset_usb_boot",
    cgi_reset_usb_boot
  }
};

u16_t ssi_handler(int iIndex, char *pcInsert, int iInsertLen){
  size_t printed;
  switch (iIndex) {
  case 0: /* "GPIO" */
    {
      printed = snprintf(pcInsert, iInsertLen, "%d", gpio_get(IN_PIN) );
    }
    break;
  case 1: /* "COUNTER" */
    {
      static int counter;
      counter++;
      printed = snprintf(pcInsert, iInsertLen, "%d", counter);
    }
    break;
  default: /* unknown tag */
    printed = 0;
    break;
  }
  LWIP_ASSERT("sane length", printed <= 0xFFFF);
  return (u16_t)printed;
}

int main()
{
  // Initialize tinyusb, lwip, dhcpd and httpd
  init_lwip();
  wait_for_netif_is_up();
  dhcpd_init();
  httpd_init();
  http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
  http_set_ssi_handler(ssi_handler, ssi_tags, LWIP_ARRAYSIZE(ssi_tags));

  // For toggle_led
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);
  gpio_put(LED_PIN, 1);

  // For input
  gpio_init(IN_PIN);
  gpio_set_dir(IN_PIN, GPIO_IN);
  // Internal pull ups.
  gpio_pull_up(IN_PIN);

  while (true)
  {
    tud_task();
    service_traffic();
  }

  return 0;
}
