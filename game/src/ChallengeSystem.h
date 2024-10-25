/************************************************************
* title_name		: Challenge System
* author			: SkyOfDance
* date				: 15.08.2022
************************************************************/
#pragma once	
#ifdef ENABLE_CHALLENGE_SYSTEM
enum ChallengeConf
{
	CHALLENGE_QUEST_NAME_1,//�lk 120. seviyeye ula�an.
	CHALLENGE_QUEST_NAME_2,//�lk 300 bin derece yapan.
	CHALLENGE_QUEST_NAME_3,//�lk 16 tip seviyeli Pet yapan.
	CHALLENGE_QUEST_NAME_4,//�lk 250 seviye Pet yapan.
	MAX_QUEST_LIMIT,
};

typedef struct SChallengeInfo
{
	BYTE bId;
	char szWinner[64 + 1];
	bool bState;
}TChallengeInfo;

class CChallenge : public singleton<CChallenge>
{
public:
	CChallenge();
	virtual ~CChallenge();

	void Initialize();
	void Destroy();
	TChallengeInfo GetInfoDB(BYTE id);
	void SetWinner(BYTE id, LPCHARACTER ch);
	void RecvWinnerList(LPCHARACTER ch);
	bool GetWinner(BYTE id);
	void RefinedChallenge(DWORD vnum, LPCHARACTER ch);
	void UpdateP2P(TChallengeInfo pgg);
protected:
	bool m_Initialize;
	TChallengeInfo m_ChallengeWinnerList[ChallengeConf::MAX_QUEST_LIMIT];


	const DWORD m_Challenge_GiftValue[ChallengeConf::MAX_QUEST_LIMIT] =
	{
		350,//�lk 120. seviyeye ula�an.
		500,//�lk 300 bin derece yapan.
		150,//�lk 16 tip seviyeli Pet yapan.
		5000,//�lk 250 seviye Pet yapan.
		200,//�lk Mor Tanr��a �temlerinden birini +9 yapan.
		225,//�lk Ye�il Tanr��a yapan.
		300,//�lk Nemea Peti+25 yapan.
		300,//�lk Nemea Bine�i+25 yapan.
		300,//�lk Nemea Kask Kost�m�+25 basan.
		300,//�lk Nemea Giysi Kost�m�+25 basan.
		300,//�lk Nemea Silah Kost�m�+25 basan.
		200,//�lk Sage seviyesinde beceri yapan.
		250,//�lk Yohara 10.seviyeye ula�an.
		450,//�lk Reborn 10.seviyeye ula�an.
		750,//�lk Astte�men r�tbesine ula�an.
		1000,//�lk Zaman t�nelinde 20.seviyeye ula�an.
		100,//�lk Nemea Da�lar� y�z�klerinden birini +9 yapan.
		300,//�lk Nemea Da�lar� Eldiveni+0 yapan.
		300,//�lk Nemea Da�lar� Kemeri+0 yapan.
		300,//�lk Element T�ls�m�+0 yapan.
		500,//�lk Biologda Sevgililer G�n� K�resi g�revini tamamlayan
		600,//�lk Nemea Peti+50 yapan.
		600,//�lk Nemea Bine�i+50 yapan.
		600,//�lk Nemea Kask Kost�m�+50 basan.
		600,//�lk Nemea Giysi Kost�m�+50 basan.
		600,//�lk Nemea Silah Kost�m�+50 basan.
		400,//�lk Ye�il Tanr��a �temlerinden birini +9 yapan.
		500,//�lk 1 milyon derece yapan.
		500,//�lk Yohara 30.seviyeye ula�an.
		300,//�lk Boosterlardan herhangi birini +9 yapan.
		300,//�lk �ebnemlerden herhangi birini +9 yapan.
		1000,//�lk 100 Emi� ku�ak yapan.
		750,//�lk Reborn 20.seviyeye ula�an.
		750,//�lk Nemea Peti+75 yapan.
		750,//�lk Nemea Bine�i+75 yapan.
		750,//�lk Nemea Kask Kost�m�+75 basan.
		750,//�lk Nemea Giysi Kost�m�+75 basan.
		750,//�lk Nemea Silah Kost�m�+75 basan.
		500,//�lk Legendary seviyesinde beceri yapan.
		750,//�lk R�ya & G�k Ruhu+100 yapan.
		500,//�lk Mavi Tanr��a yapan.
		500,//�lk Boosterlardan herhangi birini +15 yapan.
		500,//�lk �ebnemlerden herhangi birini +15 yapan.
		500,//�lk R�nlerden herhangi birini +9 yapan.
		750,//�lk Mavi Tanr��a �temlerinden birini +9 yapan.
		3000,//�lk 250 seviye Tanr��a yapan.
		100,//�lk Canavardan Koruyan Ta�+10 yapan.
		100,//�lk Metin Ta��+10 yapan.
		100,//�lk Patron Ta��+10 yapan.
		100,//�lk Savunma Ta��+10 yapan.
		1000,//�lk Zaman t�nelinde 32.seviyeye ula�an.
		1000,//�lk Reborn 30.seviyeye ula�an.
		1000,//�lk Mara�el r�tbesine ula�an.
		1000,//�lk Yohara 60.seviyeye ula�an.
		1000,//�lk Nemea Peti+100 yapan.
		1000,//�lk Nemea Bine�i+100 yapan.
		1000,//�lk Nemea Kask Kost�m�+100 basan.
		1000,//�lk Nemea Giysi Kost�m�+100 basan.
		1000,//�lk Nemea Silah Kost�m�+100 basan.
		1000,//�lk Biolog g�revlerini bitiren.
		200,//�lk Perma Kalkan Cevheri yapan.
		200,//�lk Perma Ayakkab� Cevheri yapan.
		200,//�lk Perma Kask Cevheri yapan.
		200,//�lk Perma T�ls�m Cevheri yapan.
		200,//�lk Perma Eldiven Cevheri yapan.
		100,//�lk Perma Kost�m Cevheri yapan.
		100,//�lk Perma Booster Cevheri yapan.
		100,//�lk Perma Y�z�k Cevheri yapan.
		100,//�lk Perma R�ya Ruhu Cevheri yapan.
		500,//�lk Z�mr�t Eldiveni+0 yapan.
		500,//�lk 250 Ortalama Silah yapan.
		750//�lk Element T�ls�m�+200 yapan.
	};
};
#endif