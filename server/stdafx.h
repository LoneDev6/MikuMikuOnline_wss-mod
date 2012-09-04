// stdafx.h : �W���̃V�X�e�� �C���N���[�h �t�@�C���̃C���N���[�h �t�@�C���A�܂���
// �Q�Ɖ񐔂������A�����܂�ύX����Ȃ��A�v���W�F�N�g��p�̃C���N���[�h �t�@�C��
// ���L�q���܂��B
//

#pragma once

#ifdef _WIN32
#include <tchar.h>
#endif

// C �����^�C�� �w�b�_�[ �t�@�C��
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <stdint.h>
#include <assert.h>
#include <cmath>
#include <iostream>

// TODO: �v���O�����ɕK�v�Ȓǉ��w�b�_�[�������ŎQ�Ƃ��Ă��������B

#include <vector>
#include <list>
#include <map>
#include <deque>
#include <queue>
#include <array>
#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <algorithm>

#include <pssr.h>
#include <osrng.h>
#include <modes.h>
#include <aes.h>
#include <rsa.h>
#include <sha.h>

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/timer.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/format.hpp>
#include <boost/thread.hpp>
#include <boost/foreach.hpp>

#include "../common/Logger.hpp"