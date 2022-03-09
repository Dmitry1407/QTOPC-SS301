#ifndef MAIN
#define MAIN


// {FC514528-C87A-42fa-97E5-9EE8011F10AD}
DEFINE_GUID (GID_QT_OPC_Server,
0xfc514528, 0xc87a, 0x42fa, 0x97, 0xe5, 0x9e, 0xe8, 0x1, 0x1f, 0x10, 0xad);

#define PORT_NUM_MAX		8			// максимальное количество портов
#define DEV_NUM_MAX			8			// максимальное количество устройств, подключенных к одному порту
#define TAGS_NUM_MAX		4096		// максимальное количество тегов
#define TAGS_IN_DEVICE		31			// число тегов на 1 устройства
#define DATALEN_MAX			200			// максимальная длина тега

const char driverName[] = "QTOPC-SS301";

#endif // MAIN
