#include "gattserver.h"

#include <cstring>

/*
<GATT_SERVER handle="0000" uuid="579cabf3-bfe1-4c27-84c1-4955b6d5a102">
      <SERVICE handle="0001" uuid="180F" primary="true" enable="true">
         <CHARACTERISTIC handle="0011" uuid="2901" is_descriptor="true" value="76657269666F6E652062617474657279206C6576656C" enable="true" />
         <CHARACTERISTIC handle="0012" uuid="2A19" value="20" read="true" notify="true" authentication_signed_writes="true" enable="true" />
         <CHARACTERISTIC handle="0013" uuid="2901" is_descriptor="true" value="76657269666F6E65206261747465727920737461747573" enable="true" />
         <CHARACTERISTIC handle="0014" uuid="2A1A" value="64" broadcast="true" read="true" authentication_signed_writes="true" enable="true" />
      </SERVICE>
      <SERVICE handle="0002" uuid="579cabf4-bfe1-4c27-84c1-4955b6d5a102" primary="true" enable="true">
         <CHARACTERISTIC handle="0021" uuid="2901" is_descriptor="true" value="707269766174655F64617461" enable="true" />
         <CHARACTERISTIC handle="0022" uuid="579cabf5-bfe1-4c27-84c1-4955b6d5a102" value="7465737420737472696E67" authentication_signed_reads="true" authentication_signed_writes="true" enable="true" />
         <CHARACTERISTIC handle="0023" uuid="2901" is_descriptor="true" value="7075626C69635F646174615F6E6F74696679" enable="true" />
         <CHARACTERISTIC handle="0024" uuid="579cabf7-bfe1-4c27-84c1-4955b6d5a102" value="20" read="true" notify="true" write="true" enable="true" />
         <CHARACTERISTIC handle="0025" uuid="2901" is_descriptor="true" value="7075626C69635F646174615F696E646963617465" enable="true" />
         <CHARACTERISTIC handle="0026" uuid="579cabf8-bfe1-4c27-84c1-4955b6d5a102" value="02" read="true" indicate="true" write="true" enable="true" />
      </SERVICE>
   </GATT_SERVER>
*/

void GATTServer::createTestServer()
{
	// handle = 0x0001;
	// {
	// 	uint8_t tmp[] = {0x57, 0x9c, 0xab, 0xf3, 0xbf, 0xe1, 0x4c, 0x27, 0x84, 0xc1, 0x49, 0x55, 0xb6, 0xd5, 0xa1, 0x02};
	// 	memcpy(uuid, tmp, sizeof(uuid));
	// }

	// GATTService svc;
	// svc.handle = 0x0002;
	// {
	// 	// TODO: By protocol it is allowed to set UUID as uint16, but maybe for ease we can deprecate it and allow only uuid128?
	// 	// OR at least convert it to uuid128
	// 	uint8_t tmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x0F};
	// 	memcpy(svc.uuid, tmp, sizeof(svc.uuid));
	// }
	// svc.primary = true;

	// {
	// 	GATTCharacteristic charstic;
	// 	charstic.handle = 0x0011;
	// 	uint8_t tmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x20};
	// 	memcpy(charstic.uuid, tmp, sizeof(charstic.uuid));
	// 	charstic.value = {'H', 'e', 'l', 'l', 'o'};
	// 	charstic.read = true;
	// 	charstic.notify = true;
	// 	svc.characteristics.push_back(charstic);
	// }

	// {
	// 	GATTCharacteristic charstic;
	// 	charstic.handle = 0x0012;
	// 	uint8_t tmp[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x22, 0x21};
	// 	memcpy(charstic.uuid, tmp, sizeof(charstic.uuid));
	// 	charstic.value = {'W', 'o', 'r', 'l', 'd'};
	// 	charstic.write = true;
	// 	charstic.read = true;
	// 	charstic.indicate = true;
	// 	svc.characteristics.push_back(charstic);
	// }

	// services.push_back(svc);
}