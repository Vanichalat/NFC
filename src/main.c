

#include <zephyr/kernel.h>
#include <zephyr/sys/reboot.h>

#include <nfc_t2t_lib.h>
#include <nfc/ndef/uri_msg.h>
#include <nfc/ndef/uri_rec.h>
// #include <nfc/ndef/record.h>

#include <dk_buttons_and_leds.h>


#define NFC_FIELD_LED		DK_LED1

//https://tappetownerdynamic.page.link/VXX6
//https://mrlink001.page.link/R6GT

static const uint8_t m_url[] =
    // {'/', '/', 't', 'a', 'p', 'p', 'e', 't', 'o', 'w', 'n', 'e', 'r', 'd', 'y', 'n', 'a', 'm', 'i', 'c','.', 'p', 'a', 'g','e', '.', 
	// 'l','i', 'n', 'k','/', 'v', 'x', 'x', '6'}; 
 {'h', 't', 't', 'p', 's', ':', '/', '/','m', 'r', 'l', 'i', 'n', 'k', '0', '0', '1', '.', 'p', 'a', 'g','e', '.', 'l', 'i', 'n', 'k','/', 'R', '6', 'G', 'T'}; 

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
//int nfc_ndef_uri_msg_encde(enum nfc_ndef_uri_rec_id uri_id_code,
	int nfc_ndef_uri_msg_encde( uint8_t const *const uri_data,
			    uint16_t uri_data_len,
			    uint8_t *buf,
			    uint32_t *len)
{
	int err;

	/* Create NFC NDEF message description with URI record */
	NFC_NDEF_MSG_DEF(nfc_uri_msg, 1);
	struct nfc_ndef_uri_rec_payload nfc_ndef_uri_record_payload_desc =  
	{								       
		//.uri_id_code = (NULL),			       
		.uri_data = (m_url),				       
		.uri_data_len = (sizeof(m_url))			       
	};	
	struct nfc_ndef_record_desc nfc_ndef_generic_record_desc =	    
	{								    
		.tnf = TNF_ABSOLUTE_URI,						    
		.id_length = 0,					    
		.id = NULL,						    
		.type_length = sizeof(m_url),			    
		.type = m_url,					    
		.payload_constructor  =					    
			(payload_constructor_t)nfc_ndef_uri_rec_payload_encode,	    
		.payload_descriptor = (void *) &nfc_ndef_uri_record_payload_desc	    
	};
	err = nfc_ndef_msg_record_add(&NFC_NDEF_MSG(nfc_uri_msg),
				      &nfc_ndef_generic_record_desc);
	if (err < 0) {
		return err;
	}

	if (!uri_data) {
		return -EINVAL;
	}
	/* Encode whole message into buffer */
	err = nfc_ndef_msg_encode(&NFC_NDEF_MSG(nfc_uri_msg),
				  buf,
				  len);
	return err;


}

int main(void)
{
	int err;
	size_t len = sizeof(m_ndef_msg_buf);

	// NFC_NDEF_URI_RECORD_DESC_DEF(vani,"code", data, len);
	
	// struct nfc_ndef_uri_rec_payload  vani_ndef_uri_record_payload_desc = {
	// 	uri_id_code = "code";
	// 	uri_data_len = len;
	// 	uri_data = data;
	// }
	/*pVani.uri_id_code = ""
	pVani.uri_data = "";
	pVani.uri_data_len = "";*/
	

	printk("Starting url example\n");

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

	
		// err = nfc_ndef_uri_msg_encode( NFC_URI_HTTPS,
		// 				m_url,
		// 				sizeof(m_url),
		// 				m_ndef_msg_buf,
		// 				&len);
		// 	printk("length is %d",len);
		err = nfc_ndef_uri_msg_encde(m_url,
						sizeof(m_url),
						m_ndef_msg_buf,
						&len);
			printk("length is %d",len);

		if (err < 0) {
			printk("Cannot encode message!\n");
			return err;
		}


// err = nfc_ndef_uri_rec_payload_encode();

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
