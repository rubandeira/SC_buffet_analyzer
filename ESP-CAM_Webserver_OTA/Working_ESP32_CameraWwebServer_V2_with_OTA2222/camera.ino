void camera_init(){
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Frame parameters
  //config.frame_size = FRAMESIZE_QVGA; //320x240
  //config.frame_size = FRAMESIZE_CIF; //400x296
  config.frame_size = FRAMESIZE_VGA; //640x480
  //config.frame_size = FRAMESIZE_SVGA; //for 800x600
  //config.frame_size = FRAMESIZE_XGA; //for 1024x768
  //config.frame_size = FRAMESIZE_SXGA; //for 1280x1024
  //config.frame_size = FRAMESIZE_UXGA; //for 1600x1200
  config.jpeg_quality = 27; //10-63 lower number means higher quality
  config.fb_count = 1; //Framebuffer count 1-2 (1 for lower quality)

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("[CAMERA] init failed with error 0x%x", err);
    return;
  }

//MORE CAMERA SETTINGS:
sensor_t * s = esp_camera_sensor_get();
s->set_brightness(s, 0);     // -2 to 2
s->set_contrast(s, 0);       // -2 to 2
s->set_saturation(s, 0);     // -2 to 2
s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
s->set_aec2(s, 0);           // 0 = disable , 1 = enable
s->set_ae_level(s, 0);       // -2 to 2
s->set_aec_value(s, 300);    // 0 to 1200
s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
s->set_agc_gain(s, 0);       // 0 to 30
s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
s->set_bpc(s, 0);            // 0 = disable , 1 = enable
s->set_wpc(s, 1);            // 0 = disable , 1 = enable
s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
s->set_lenc(s, 1);           // 0 = disable , 1 = enable
s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
s->set_vflip(s, 0);          // 0 = disable , 1 = enable
s->set_dcw(s, 1);            // 0 = disable , 1 = enable
s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

}

/*
The following table shows each additional camera setting and the values accepted:

Function	         Meaning	        Values
set_brightness()	Set brightness	  -2 to 2
set_contrast()	   Set contrast	   -2 to 2
set_saturation()	Set saturation	  -2 to 2

set_special_effect()	Set a special effect	
0 – No Effect
1 – Negative
2 – Grayscale
3 – Red Tint
4 – Green Tint
5 – Blue Tint
6 – Sepia

set_whitebal()	    Set white balance	        0 – disable 1 – enable
set_awb_gain()	    Set white balance gain	  0 – disable 1 – enable

set_wb_mode()	    Set white balance mode	
0 – Auto
1 – Sunny
2 – Cloudy
3 – Office
4 – Home

set_exposure_ctrl()	Set exposure control	    0 – disable 1 – enable
set_aec2()		                                 0 – disable 1 – enable
set_ae_level()		                              -2 to 2
set_aec_value()		                             0 to 1200
set_gain_ctrl()		                             0 – disable 1 – enable
set_agc_gain()		                               0 to 30
set_gainceiling()		                            0 to 6
set_bpc()		                                   0 – disable 1 – enable  
set_wpc()		                                   0 – disable 1 – enable
set_raw_gma()		                               0 – disable 1 – enable
set_lenc()	   Set lens correction	             0 – disable 1 – enable
set_hmirror()	Horizontal mirror	                0 – disable 1 – enable
set_vflip()	   Vertical flip	                   0 – disable 1 – enable
set_dcw()		                                    0 – disable 1 – enable
set_colorbar()	 Set a colorbar	                  0 – disable 1 – enable
*/
