/*
* CoolQ SDK for VC++ 
* Api Version 9
* Written by Coxxs & Thanks for the help of orzFly
*/
#pragma once

#define CQAPIVER 9
#define CQAPIVERTEXT "9"


#ifndef CQAPI
#define CQAPI(ReturnType) extern "C" __declspec(dllimport) ReturnType __stdcall
#endif



#define CQEVENT(ReturnType, Name, Size) __pragma(comment(linker, "/EXPORT:" #Name "=_" #Name "@" #Size))\
 extern "C" __declspec(dllexport) ReturnType __stdcall Name

typedef int32_t CQBOOL;

#define EVENT_IGNORE 0          //�¼�_����
#define EVENT_BLOCK 1           //�¼�_����

#define REQUEST_ALLOW 1         //����_ͨ��
#define REQUEST_DENY 2          //����_�ܾ�

#define REQUEST_GROUPADD 1      //����_Ⱥ���
#define REQUEST_GROUPINVITE 2   //����_Ⱥ����

#define CQLOG_DEBUG 0           //���� ��ɫ
#define CQLOG_INFO 10           //��Ϣ ��ɫ
#define CQLOG_INFOSUCCESS 11    //��Ϣ(�ɹ�) ��ɫ
#define CQLOG_INFORECV 12       //��Ϣ(����) ��ɫ
#define CQLOG_INFOSEND 13       //��Ϣ(����) ��ɫ
#define CQLOG_WARNING 20        //���� ��ɫ
#define CQLOG_ERROR 30          //���� ��ɫ
#define CQLOG_FATAL 40          //�������� ���

#ifndef TEST_BANGUMI
/*
* ����˽����Ϣ, �ɹ�������ϢID
* QQID Ŀ��QQ��
* msg ��Ϣ����
*/
CQAPI(int32_t) CQ_sendPrivateMsg(int32_t AuthCode, int64_t QQID, const char *msg);

/*
* ����Ⱥ��Ϣ, �ɹ�������ϢID
* groupid Ⱥ��
* msg ��Ϣ����
*/
CQAPI(int32_t) CQ_sendGroupMsg(int32_t AuthCode, int64_t groupid, const char *msg);

/*
* ������������Ϣ, �ɹ�������ϢID
* discussid �������
* msg ��Ϣ����
*/
CQAPI(int32_t) CQ_sendDiscussMsg(int32_t AuthCode, int64_t discussid, const char *msg);
#else
/*
* ����˽����Ϣ, �ɹ�������ϢID
* QQID Ŀ��QQ��
* msg ��Ϣ����
*/
void CQ_sendPrivateMsg(int32_t AuthCode, int64_t QQID, const char *msg) {
	std::cout << "\n[MSG-->]["
		<< QQID
		<< "]:\n********\n"
		<< msg
		<< "\n********\n"
		<<std::endl;
}

/*
* ����Ⱥ��Ϣ, �ɹ�������ϢID
* groupid Ⱥ��
* msg ��Ϣ����
*/
void CQ_sendGroupMsg(int32_t AuthCode, int64_t groupid, const char *msg) {
	CQ_sendPrivateMsg(AuthCode, groupid, msg);
}

/*
* ������������Ϣ, �ɹ�������ϢID
* discussid �������
* msg ��Ϣ����
*/
void CQ_sendDiscussMsg(int32_t AuthCode, int64_t discussid, const char *msg) {
	CQ_sendPrivateMsg(AuthCode, discussid, msg);;
}
#endif
/*
* ������Ϣ
* msgid ��ϢID
*/
CQAPI(int32_t) CQ_deleteMsg(int32_t AuthCode, int64_t msgid);

/*
* ������ �����ֻ���
* QQID QQ��
*/
CQAPI(int32_t) CQ_sendLike(int32_t AuthCode, int64_t QQID);

/*
* ��ȺԱ�Ƴ�
* groupid Ŀ��Ⱥ
* QQID QQ��
* rejectaddrequest ���ٽ��մ��˼�Ⱥ���룬������
*/
CQAPI(int32_t) CQ_setGroupKick(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL rejectaddrequest);

/*
* ��ȺԱ����
* groupid Ŀ��Ⱥ
* QQID QQ��
* duration ���Ե�ʱ�䣬��λΪ�롣���Ҫ�����������д0��
*/
CQAPI(int32_t) CQ_setGroupBan(int32_t AuthCode, int64_t groupid, int64_t QQID, int64_t duration);

/*
* ��Ⱥ����Ա
* groupid Ŀ��Ⱥ
* QQID QQ��
* setadmin true:���ù���Ա false:ȡ������Ա
*/
CQAPI(int32_t) CQ_setGroupAdmin(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL setadmin);

/*
* ��ȫȺ����
* groupid Ŀ��Ⱥ
* enableban true:���� false:�ر�
*/
CQAPI(int32_t) CQ_setGroupWholeBan(int32_t AuthCode, int64_t groupid, CQBOOL enableban);

/*
* ������ȺԱ����
* groupid Ŀ��Ⱥ
* anomymous Ⱥ��Ϣ�¼��յ��� anomymous ����
* duration ���Ե�ʱ�䣬��λΪ�롣��֧�ֽ����
*/
CQAPI(int32_t) CQ_setGroupAnonymousBan(int32_t AuthCode, int64_t groupid, const char *anomymous, int64_t duration);

/*
* ��Ⱥ��������
* groupid Ŀ��Ⱥ
* enableanomymous true:���� false:�ر�
*/
CQAPI(int32_t) CQ_setGroupAnonymous(int32_t AuthCode, int64_t groupid, CQBOOL enableanomymous);

/*
* ��Ⱥ��Ա��Ƭ
* groupid Ŀ��Ⱥ
* QQID Ŀ��QQ
* newcard ����Ƭ(�ǳ�)
*/
CQAPI(int32_t) CQ_setGroupCard(int32_t AuthCode, int64_t groupid, int64_t QQID, const char *newcard);

/*
* ��Ⱥ�˳� ����, �˽ӿ���Ҫ�ϸ���Ȩ
* groupid Ŀ��Ⱥ
* isdismiss �Ƿ��ɢ true:��ɢ��Ⱥ(Ⱥ��) false:�˳���Ⱥ(����Ⱥ��Ա)
*/
CQAPI(int32_t) CQ_setGroupLeave(int32_t AuthCode, int64_t groupid, CQBOOL isdismiss);

/*
* ��Ⱥ��Աר��ͷ�� ��Ⱥ��Ȩ��
* groupid Ŀ��Ⱥ
* QQID Ŀ��QQ
* newspecialtitle ͷ�Σ����Ҫɾ����������գ�
* duration ר��ͷ����Ч�ڣ���λΪ�롣���������Ч��������д-1��
*/
CQAPI(int32_t) CQ_setGroupSpecialTitle(int32_t AuthCode, int64_t groupid, int64_t QQID, const char *newspecialtitle, int64_t duration);

/*
* ���������˳�
* discussid Ŀ���������
*/
CQAPI(int32_t) CQ_setDiscussLeave(int32_t AuthCode, int64_t discussid);

/*
* �ú����������
* responseflag �����¼��յ��� responseflag ����
* responseoperation REQUEST_ALLOW �� REQUEST_DENY
* remark ��Ӻ�ĺ��ѱ�ע
*/
CQAPI(int32_t) CQ_setFriendAddRequest(int32_t AuthCode, const char *responseflag, int32_t responseoperation, const char *remark);

/*
* ��Ⱥ�������
* responseflag �����¼��յ��� responseflag ����
* requesttype���������¼������������� REQUEST_GROUPADD �� REQUEST_GROUPINVITE
* responseoperation  REQUEST_ALLOW �� REQUEST_DENY
* reason �������ɣ��� REQUEST_GROUPADD �� REQUEST_DENY ʱ����
*/
CQAPI(int32_t) CQ_setGroupAddRequestV2(int32_t AuthCode, const char *responseflag, int32_t requesttype, int32_t responseoperation, const char *reason);

/*
* ȡȺ��Ա��Ϣ
* groupid Ŀ��QQ����Ⱥ
* QQID Ŀ��QQ��
* nocache ��ʹ�û���
*/
CQAPI(const char *) CQ_getGroupMemberInfoV2(int32_t AuthCode, int64_t groupid, int64_t QQID, CQBOOL nocache);

/*
* ȡİ������Ϣ
* QQID Ŀ��QQ
* nocache ��ʹ�û���
*/
CQAPI(const char *) CQ_getStrangerInfo(int32_t AuthCode, int64_t QQID, CQBOOL nocache);

#ifndef TEST_BANGUMI
/*
* ��־
* priority ���ȼ���CQLOG ��ͷ�ĳ���
* category ����
* content ����
*/
CQAPI(int32_t) CQ_addLog(int32_t AuthCode, int32_t priority, const char *category, const char *content);
#else
void CQ_addLog(int32_t AuthCode, int32_t priority, const char *category, const char *content)
{
	std::cout << "["
		<< category
		<< "] "
		<< content
		<< std::endl;
}
#endif

/*
* ȡCookies ����, �˽ӿ���Ҫ�ϸ���Ȩ
*/
CQAPI(const char *) CQ_getCookies(int32_t AuthCode);

/*
* ȡCsrfToken ����, �˽ӿ���Ҫ�ϸ���Ȩ
*/
CQAPI(int32_t) CQ_getCsrfToken(int32_t AuthCode);

/*
* ȡ��¼QQ
*/
CQAPI(int64_t) CQ_getLoginQQ(int32_t AuthCode);

/*
* ȡ��¼QQ�ǳ�
*/
CQAPI(const char *) CQ_getLoginNick(int32_t AuthCode);


#ifndef TEST_BANGUMI
/*
* ȡӦ��Ŀ¼�����ص�·��ĩβ��"\"
*/
CQAPI(const char *) CQ_getAppDirectory(int32_t AuthCode);
#else
const char * CQ_getAppDirectory(int32_t AuthCode)
{
	return ".\\";
}
#endif
/*
* ������������ʾ
* errorinfo ������Ϣ
*/
CQAPI(int32_t) CQ_setFatal(int32_t AuthCode, const char *errorinfo);

/*
* ����������������Ϣ�е�����(record),���ر����� \data\record\ Ŀ¼�µ��ļ���
* file �յ���Ϣ�е������ļ���(file)
* outformat Ӧ������������ļ���ʽ��Ŀǰ֧�� mp3 amr wma m4a spx ogg wav flac
*/
CQAPI(const char *) CQ_getRecord(int32_t AuthCode, const char *file, const char *outformat);

int ac = -1; //AuthCode ���ÿ�Q�ķ���ʱ��Ҫ�õ�
