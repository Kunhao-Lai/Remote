#pragma once



enum
{
	// �ļ����䷽ʽ
	TRANSFER_MODE_NORMAL = 0x00,	// һ��,������ػ���Զ���Ѿ��У�ȡ��
	TRANSFER_MODE_ADDITION,			// ׷��
	TRANSFER_MODE_ADDITION_ALL,		// ȫ��׷��
	TRANSFER_MODE_OVERWRITE,		// ����
	TRANSFER_MODE_OVERWRITE_ALL,	// ȫ������
	TRANSFER_MODE_JUMP,				// ����
	TRANSFER_MODE_JUMP_ALL,			// ȫ������
	TRANSFER_MODE_CANCEL,			// ȡ������
};
enum
{
	CLIENT_LOGIN,                      //���߰�
	CLIENT_GET_OUT_REQUEST,              //	ǿ�������û�����
	CLIENT_GET_OUT_REPLY,                //ǿ�������û��ظ�
	CLIENT_REMOTE_MESSAGE_REQUEST,       //��ʱ��Ϣ����
	CLIENT_REMOTE_MESSAGE_REPLY,         //��ʱ��Ϣ�ظ�
	CLIENT_REMOTE_MESSAGE_COMPLETE,
	CLIENT_SHUT_DOWN_REQUEST,
	CLIENT_SHUT_DOWN_REPLY,

	CLIENT_CMD_MANAGER_REQUIRE,
	CLIENT_CMD_MANAGER_REPLY,
	CLIENT_PROCESS_MANAGER_REQUIRE,
	CLIENT_PROCESS_MANAGER_REPLY,
	CLIENT_PROCESS_REFRESH_REQUIRE,
	CLIENT_PROCESS_REFRESH_REPLY,
	CLIENT_PROCESS_TERMINATE_REQUIRE,
	CLIENT_PROCESS_THREAD_INFO_SHOW_REQUIRE,
	CLIENT_PROCESS_THREAD_INFO_SHOW_REPLY,
	CLIENT_PROCESS_HANDLE_INFO_SHOW_REQUIRE,
	CLIENT_PROCESS_HANDLE_INFO_SHOW_REPLY,
	CLIENT_WINDOW_MANAGER_REQUIRE,
	CLIENT_WINDOW_MANAGER_REPLY,
	CLIENT_WINDOW_REFRESH_REQUIRE,
	CLIENT_REMOTE_CONTROL_REQUIRE,
	CLIENT_REMOTE_CONTROL_REPLY,
	CLIENT_REMOTE_CONTROL_FIRST_SCREEN,
	CLIENT_REMOTE_CONTROL_NEXT_SCREEN,
	CLIENT_REMOTE_CONTROL_CONTROL_REQUIRE,
	CLIENT_REMOTE_CONTROL_BLOCK_INPUT,
	CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE,
	CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REPLY,
	CLIENT_REMOTE_CONTROL_SET_CLIPBOARD_REQUIRE,
	CLIENT_REMOTE_CONTROL_SET_CLIPBOARD_REPLY,
	CLIENT_FILE_MANAGER_REQUIRE,
	CLIENT_FILE_MANAGER_REPLY,
	CLIENT_FILE_MANAGER_FILE_DATA_REQUIRE,
	CLIENT_FILE_MANAGER_FILE_DATA_REPLY,
	CLIENT_FILE_MANAGER_DELETE_FILE_REQUEST,
	CLIENT_FILE_MANAGER_NEW_FOLDER_REQUEST,
	CLIENT_FILE_MANAGER_FILE_INFORMATION_REQUEST,
	CLIENT_FILE_MANAGER_TRANSFER_MODE_REQUEST,
	CLIENT_FILE_MANAGER_TRANSFER_MODE_REPLY,
	CLIENT_FILE_MANAGER_FILE_DATA,
	CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE,
	CLIENT_AUDIO_MANAGER_REQUIRE,
	CLIENT_AUDIO_MANAGER_REPLY,
	CLIENT_AUDIO_MANAGER_DATA,
	CLIENT_SERVICE_MANAGER_REQUIRE,
	CLIENT_SERVICE_MANAGER_REPLY,
	CLIENT_SERVICE_MANAGER_CONFIG_REQUIRE,
	CLIENT_REGISTER_MANAGER_REQUIRE,
	CLIENT_REGISTER_MANAGER_REPLY,
	CLIENT_REGISTER_MANAGER_DATA,
	CLIENT_REGISTER_MANAGER_PATH_DATA_REPLY,
	CLIENT_REGISTER_MANAGER_KEY_DATA_REPLY,
	CLIENT_GO_ON,

};