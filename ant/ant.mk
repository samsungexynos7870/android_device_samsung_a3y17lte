# ANT+ (com.qualcomm.qti.ant@1.0::IAntHci) built from source for the
# open-source QCA9377 stack. Enabled via BOARD_ANT_WIRELESS_DEVICE
# (BoardConfig.mk), which also makes libbt-vendor compile its ANT channel
# (-DENABLE_ANT: BT_VND_OP_ANT_USERIAL_{OPEN,CLOSE} -> ant_sock on the
# wcnss_filter mux daemon).
PRODUCT_PACKAGES += \
    com.qualcomm.qti.ant@1.0 \
    com.qualcomm.qti.ant@1.0-impl \
    com.qualcomm.qti.ant@1.0-service \
    libnativehelper_shim
