

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#include <nfc_t2t_lib.h>
#include <nfc/ndef/uri_msg.h>
#include <nfc/ndef/uri_rec.h>

#include <dk_buttons_and_leds.h>


#define NFC_FIELD_LED		DK_LED1


static const uint8_t m_url[] =
    {'g', 'o', 'o', 'g', 'l', 'e', '.', 'c', 'o', 'm'}; //URL "google.com"

uint8_t m_ndef_msg_buf[256];


static void nfc_callback(void *context,
			 nfc_t2t_event_t event,
			 const uint8_t *data,
			 size_t data_length)
{
	ARG_UNUSED(context);
	ARG_UNUSED(data);
	ARG_UNUSED(data_length);

	switch (event) {
	case NFC_T2T_EVENT_FIELD_ON:
		dk_set_led_on(NFC_FIELD_LED);
		break;
	case NFC_T2T_EVENT_FIELD_OFF:
		dk_set_led_off(NFC_FIELD_LED);
		break;
	default:
		break;
	}
}

int main(void)
{
	int err;
	size_t len = sizeof(m_ndef_msg_buf);

	printk("Starting NFC Launch app example\n");

	/* Configure LED-pins as outputs */
	err = dk_leds_init();
	if (err) {
		printk("Cannot init LEDs!\n");
		goto fail;
	}

	/* Set up NFC */
	err = nfc_t2t_setup(nfc_callback, NULL);
	if (err) {
		printk("Cannot setup NFC T2T library!\n");
		goto fail;
	}

	
		err = nfc_ndef_uri_msg_encode( NFC_URI_HTTP_WWW,
						m_url,
						sizeof(m_url),
						m_ndef_msg_buf,
						&len);
			printk("length is %d",len);

		if (err < 0) {
			printk("Cannot encode message!\n");
			return err;
		}



	err = nfc_t2t_payload_set(m_ndef_msg_buf, len);
	if (err) {
		printk("Cannot set payload!\n");
		goto fail;
	}

	/* Start sensing NFC field */
	err = nfc_t2t_emulation_start();
	if (err) {
		printk("Cannot start emulation!\n");
		goto fail;
	}

	printk("NFC configuration done\n");
	return 0;

fail:
#if CONFIG_REBOOT
	sys_reboot(SYS_REBOOT_COLD);
#endif /* CONFIG_REBOOT */

	return err;
}
