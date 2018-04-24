#include <stdlib.h>
#include <string.h>
#include "nvm_manager.h"

#include "hwcrypto/sha.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "esp_log.h"

#include "include/bluetooth_config.h"

#define STORAGE_NAMESPACE "storage"
#define NVM_MANAGER_KEY "NVM_MAN"
#define AUTH_COUNTER_CHANGE_PER_FLASH_WRITE 256

typedef enum
{
	NVM_ERROR_FLASH_ERROR,
	NVM_ERROR_NO_DATA_FOUND,
	NVM_ERROR_WRONG_DATA_SIZE,
	NVM_ERROR_BAD_SAVED_DATA,
	NVM_SUCCESS
} nvm_err_t;

typedef struct
{
	char descriptor_string[64];
	char password[MAX_PASSWORD_LENGTH];
	uint64_t auth_counter;
	uint8_t sha256[32];  //always at end
} _nvm_struct;

uint16_t _auth_counter_change = 0;
_nvm_struct _nvm_cache;

nvm_err_t _ReadFlash(void)
{
	nvs_handle my_handle;
    esp_err_t err;

 	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
	{
		ESP_LOGE("NVM_MANAGER", "Error 1\n");
		return NVM_ERROR_FLASH_ERROR;
	}

	// Read the size of memory space required for blob
    size_t last_stored_size = sizeof(_nvm_struct);  // value will default to 0, if not set yet in NVS
    err = nvs_get_blob(my_handle, NVM_MANAGER_KEY, (void*)&_nvm_cache, &last_stored_size);

    if(err != ESP_OK || last_stored_size != sizeof(_nvm_struct))
    {
    	ESP_LOGE("NVM_MANAGER", "Error 2, %d\n", (uint8_t)err);
    	//nothing has been stored yet or size was mismatched, clear it all and return;
    	memset(&_nvm_cache, 0, sizeof(_nvm_struct));
    	return NVM_ERROR_WRONG_DATA_SIZE;
    }

    //compute sha to ensure saved data matches what was last computed
    uint8_t computed_sha[32];
    esp_sha(SHA2_256, (void*)&_nvm_cache, (sizeof(_nvm_struct)-32), computed_sha); //size minus 32 because sha is not computed including sha
    if(memcmp(_nvm_cache.sha256, computed_sha, 32) != 0)
    {
    	ESP_LOGE("NVM_MANAGER", "Error 3\n");
    	memset(&_nvm_cache, 0, sizeof(_nvm_struct));
    	return NVM_ERROR_BAD_SAVED_DATA;
    }
    return NVM_SUCCESS;
}

nvm_err_t _WriteFlash(void)
{
	nvs_handle my_handle;
    esp_err_t err;

    //re-calculate sha
    esp_sha(SHA2_256, (void*)&_nvm_cache, (sizeof(_nvm_struct)-32), _nvm_cache.sha256);

 	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
	{
		ESP_LOGE("NVM_MANAGER", "Error 4\n");
		return NVM_ERROR_FLASH_ERROR;
	}

	err = nvs_set_blob(my_handle, NVM_MANAGER_KEY, (void*)&_nvm_cache, sizeof(_nvm_struct));

	// Commit
    err = nvs_commit(my_handle);
    if (err != ESP_OK)
    {
    	ESP_LOGE("NVM_MANAGER", "Error 5\n");
    	return NVM_ERROR_FLASH_ERROR;
    }

    // Close
    nvs_close(my_handle);
    ESP_LOGE("NVM_MANAGER", "SUCCESS 2\n");
    _auth_counter_change = 0;
    return NVM_SUCCESS;
}

void NvmManager_Create(void)
{
	nvs_flash_init();

	//try to load the data
	_ReadFlash();
    
    //increment by max amount that last auth counter could have been incremented in case of power failure/lockup etc.
    _nvm_cache.auth_counter += AUTH_COUNTER_CHANGE_PER_FLASH_WRITE;

}

void NvmManager_WriteCache(void)
{
	_WriteFlash();
}

uint64_t NvmManager_GetCachedAuthCounter(void)
{
	return _nvm_cache.auth_counter;
}

void NvmManager_IncrementAuthCounter(void)
{
	_nvm_cache.auth_counter++;
    _auth_counter_change++;
    if(_auth_counter_change >= AUTH_COUNTER_CHANGE_PER_FLASH_WRITE)
    {
        _WriteFlash();
    }
}

uint8_t NvmManager_GetPassword(char* password)
{
	if(password)
	{
		memcpy(password, _nvm_cache.password, MAX_PASSWORD_LENGTH);
		return strlen(_nvm_cache.password);
	}
	return 0;
}