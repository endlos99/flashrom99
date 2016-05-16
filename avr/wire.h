// wire.h

void disableBus(void);
void enableBus(void);

void setRAM(uint16_t addr, uint8_t byte);

void receiveData(uint8_t *buffer);

void flash_error(const uint8_t count);


#ifdef DEBUG
void show(uint8_t n);
#endif
