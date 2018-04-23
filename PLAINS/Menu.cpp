#include <fstream>

#include "Menu.h"
#include "Controls.h"
#include "Hooks.h" 
#include "Interfaces.h"
#include "XorStr.hpp"
#include "mac.h"

#define WINDOW_WIDTH 659
#define WINDOW_HEIGHT 808

PLAINS Menu::Window;

void KnifeApplyCallbk()
{
	static ConVar* Update = m_pCVar->FindVar(XorStr("cl_fullupdate"));
	Update->nFlags &= ~FCVAR_CHEAT;
	m_pEngine->ClientCmd_Unrestricted(XorStr("cl_fullupdate"));
}

struct Config_t
{
	int id;
	std::string name;
};

std::vector<Config_t> configs;

void save_callback()
{
	int should_save = Menu::Window.MiscTab.ConfigListBox.GetIndex();
	std::string config_directory = "PLAINS\\";
	config_directory += configs[should_save].name; config_directory += ".xml";
	GUI.SaveWindowState(&Menu::Window, XorStr(config_directory.c_str()));
}

void load_callback()
{
	int should_load = Menu::Window.MiscTab.ConfigListBox.GetIndex();
	std::string config_directory = "PLAINS\\";
	config_directory += configs[should_load].name; config_directory += ".xml";
	GUI.LoadWindowState(&Menu::Window, XorStr(config_directory.c_str()));
}

void list_configs()
{
	configs.clear();
	Menu::Window.MiscTab.ConfigListBox.ClearItems();

	std::ifstream file_in;
	file_in.open("PLAINS\\config.txt");

	if(file_in.fail())
	{
		std::ofstream("PLAINS\\config.txt");
		file_in.open("PLAINS\\config.txt");
	}

	int line_count;

	while(!file_in.eof())
	{
		Config_t config;
		file_in >> config.name;
		config.id = line_count;
		configs.push_back(config);

		line_count++;

		Menu::Window.MiscTab.ConfigListBox.AddItem(config.name);
	}

	file_in.close();

	if(configs.size() > 7) Menu::Window.MiscTab.ConfigListBox.AddItem(" ");
}

void add_config()
{
	std::fstream file;
	file.open("PLAINS\\config.txt", std::fstream::app);

	if(file.fail())
	{
		std::fstream("PLAINS\\config.txt");
		file.open("PLAINS\\config.txt", std::fstream::app);
	}

	file << std::endl << Menu::Window.MiscTab.NewConfigName.getText();

	file.close();

	list_configs();

	int should_add = Menu::Window.MiscTab.ConfigListBox.GetIndex();
	std::string config_directory = "PLAINS\\";
	config_directory += Menu::Window.MiscTab.NewConfigName.getText(); config_directory += ".xml";
	GUI.SaveWindowState(&Menu::Window, XorStr(config_directory.c_str()));

	Menu::Window.MiscTab.NewConfigName.SetText("");
}

void remove_config()
{
	int should_remove = Menu::Window.MiscTab.ConfigListBox.GetIndex();

	std::string config_directory = "PLAINS\\";
	config_directory += configs[should_remove].name; config_directory += ".xml";
	std::remove(config_directory.c_str());

	std::ofstream ofs("PLAINS\\config.txt", std::ios::out | std::ios::trunc);
	ofs.close();

	std::fstream file;
	file.open("PLAINS\\config.txt", std::fstream::app);

	if(file.fail())
	{
		std::fstream("PLAINS\\config.txt");
		file.open("PLAINS\\config.txt", std::fstream::app);
	}

	for(int i = 0; i < configs.size(); i++)
	{
		if(i == should_remove) continue;
		Config_t config = configs[i];
		file << std::endl << config.name;
	}

	file.close();

	list_configs();
}

void UnLoadCallbk()
{
	DoUnload = true;
}

void PLAINS::Setup()
{
	SetPosition(100, 49);
	SetSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	SetTitle(XorStr("PLAINS"));

	RegisterTab(&RageBotTab);
	RegisterTab(&LegitTab);
	RegisterTab(&VisualsTab);
	RegisterTab(&MiscTab);
	RegisterTab(&SkinsTab);
	RegisterTab(&PlayersTab);

	RECT Client = GetClientArea();
	Client.bottom -= 29;

	RageBotTab.Setup();
	LegitTab.Setup();
	VisualsTab.Setup();
	MiscTab.Setup();
	SkinsTab.Setup();
	PlayersTab.Setup();
}

void CRageBotTab::Setup()
{
	SetTitle(XorStr("a"));

	//Aimbot
	AimbotGroup.SetText(XorStr("Aimbot"));
	AimbotGroup.SetSize(293, 540);
	AimbotGroup.SetPosition(24, 16);
	AimbotGroup.AddTab(CGroupTab("Simple", 1));
	AimbotGroup.AddTab(CGroupTab("Advanced", 2));
	AimbotGroup.AddTab(CGroupTab("Hitscan", 3));
	RegisterControl(&AimbotGroup);

	Enabled.SetFileId(XorStr("r_enabled"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Activate"), this, &Enabled);

	AimbotMode.SetFileId(XorStr("r_aimbotmode"));
	AimbotMode.AddItem(XorStr("Visualized"));
	AimbotMode.AddItem(XorStr("Non-visualized"));
	AimbotMode.AddItem(XorStr("Anti-aim"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Aimbot mode"), this, &AimbotMode);

	TargetSelection.SetFileId(XorStr("r_selection"));
	TargetSelection.AddItem(XorStr("FOV"));
	TargetSelection.AddItem(XorStr("Distance"));
	TargetSelection.AddItem(XorStr("Lowest health"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Selection type"), this, &TargetSelection);

	FriendlyFire.SetFileId(XorStr("r_friendlyfire"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Friendly fire"), this, &FriendlyFire);

	AutoFire.SetFileId(XorStr("r_autofire"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Auto fire"), this, &AutoFire);

	AutoFireKey.SetFileId(XorStr("r_autofirekey"));
	AimbotGroup.PlaceLabeledControl(1, XorStr(""), this, &AutoFireKey);

	AutoFireMode.SetFileId(XorStr("r_autofiremode"));
	AutoFireMode.AddItem(XorStr("Bone"));
	AutoFireMode.AddItem(XorStr("Key Press"));
	AimbotGroup.PlaceLabeledControl(1, XorStr(""), this, &AutoFireMode);

	AutoFireRevolverMode.SetFileId(XorStr("r_autofirerevolver"));
	AutoFireRevolverMode.AddItem(XorStr("Off"));
	AutoFireRevolverMode.AddItem(XorStr("Primary"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Revolver shoot type"), this, &AutoFireRevolverMode);

	AutoFireTarget.SetFileId(XorStr("r_autofiretarget"));
	AutoFireTarget.AddItem(XorStr("Head"));
	AutoFireTarget.AddItem(XorStr("Neck"));
	AutoFireTarget.AddItem(XorStr("Chest"));
	AutoFireTarget.AddItem(XorStr("Stomach"));
	AutoFireTarget.AddItem(XorStr("Pelvis"));
	AutoFireTarget.AddItem(XorStr("Left arm"));
	AutoFireTarget.AddItem(XorStr("Right arm"));
	AutoFireTarget.AddItem(XorStr("Left leg"));
	AutoFireTarget.AddItem(XorStr("Right leg"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Autofire bone"), this, &AutoFireTarget);

	MaximumFov.SetFileId(XorStr("r_fieldofview"));
	MaximumFov.SetBoundaries(0, 180);
	MaximumFov.extension = XorStr("°");
	MaximumFov.SetValue(0);
	AimbotGroup.PlaceLabeledControl(1, XorStr("Limit FOV"), this, &MaximumFov);

	AutoWall.SetFileId(XorStr("r_autowall"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Automatic penetration"), this, &AutoWall);

	MinimumDamage.SetFileId(XorStr("r_mindmg"));
	MinimumDamage.SetBoundaries(1, 100);
	MinimumDamage.SetValue(1);
	AimbotGroup.PlaceLabeledControl(1, XorStr("Minimum damage"), this, &MinimumDamage);

	AimStep.SetFileId(XorStr("r_aimstep"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Aim step"), this, &AimStep);

	PreferBaim.SetFileId(XorStr("r_preferbaim"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Prefer body"), this, &PreferBaim);

	BaimOnXHealth.SetFileId(XorStr("r_baimonxhp"));
	BaimOnXHealth.SetBoundaries(0, 100);
	BaimOnXHealth.SetValue(0);
	AimbotGroup.PlaceLabeledControl(2, XorStr("Body aim on x hp"), this, &BaimOnXHealth);

	BaimIfDeadly.SetFileId(XorStr("r_baimifdeadly"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Body aim if deadly"), this, &BaimIfDeadly);

	BodyAimAwp.SetFileId(XorStr("r_bodyaimawp"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Body aim with awp"), this, &BodyAimAwp);

	BodyAimAwpMode.SetFileId(XorStr("r_bodyaimawpmode"));
	BodyAimAwpMode.AddItem(XorStr("Always"));
	BodyAimAwpMode.AddItem(XorStr("Prefer"));
	AimbotGroup.PlaceLabeledControl(2, XorStr(""), this, &BodyAimAwpMode);

	BodyAimScout.SetFileId(XorStr("r_bodyaimscout"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Body aim with scout"), this, &BodyAimScout);

	BodyAimScoutMode.SetFileId(XorStr("r_bodyaimscoutmode"));
	BodyAimScoutMode.AddItem(XorStr("Always"));
	BodyAimScoutMode.AddItem(XorStr("Prefer"));
	AimbotGroup.PlaceLabeledControl(2, XorStr(""), this, &BodyAimScoutMode);

	Pointscale.SetFileId(XorStr("r_pointscale"));
	Pointscale.SetBoundaries(0, 100);
	Pointscale.extension = XorStr("%%");
	Pointscale.SetValue(90);
	AimbotGroup.PlaceLabeledControl(2, XorStr("Pointscale"), this, &Pointscale);

	MinimumHitChance.SetFileId(XorStr("r_minhitchance"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Minimum hit chance"), this, &MinimumHitChance);

	MinimumHitChanceAmount.SetFileId(XorStr("r_minhitchanceamount"));
	MinimumHitChanceAmount.SetBoundaries(0, 100);
	MinimumHitChanceAmount.extension = XorStr("%%");
	MinimumHitChanceAmount.SetValue(0);
	AimbotGroup.PlaceLabeledControl(2, XorStr(""), this, &MinimumHitChanceAmount);

	AutomaticScope.SetFileId(XorStr("r_autoscope"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Automatic scope"), this, &AutomaticScope);

	Autostop.SetFileId(XorStr("r_autostop"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Automatic stop"), this, &Autostop);

	AutostopType.SetFileId(XorStr("r_autostoptype"));
	AutostopType.AddItem(XorStr("Simple"));
	AutostopType.AddItem(XorStr("Trigonometry"));
	AimbotGroup.PlaceLabeledControl(2, XorStr(""), this, &AutostopType);

	AccuracyNotCrouching.SetFileId(XorStr("r_accuracynocrouch"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Accuracy while standing"), this, &AccuracyNotCrouching);

	RemoveRecoil.SetFileId(XorStr("r_removerecoil"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Remove recoil"), this, &RemoveRecoil);

	HitscanBones.SetFileId(XorStr("r_hitscanbones"));
	HitscanBones.SetTitle(XorStr("Bones"));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Head")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Neck")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Lower neck")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Upper chest")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Chest")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Lower chest")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Stomach")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Pelvis")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Left upper arm")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Left lower arm")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Left hand")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Right upper arm")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Right lower arm")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Right hand")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Left thigh")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Left shin")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Left foot")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Right thigh")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Right shin")));
	HitscanBones.items.push_back(MultiBoxItem(false, XorStr("Right foot")));
	AimbotGroup.PlaceLabeledControl(3, XorStr(""), this, &HitscanBones);

	//Resolver
	ResolverOptions.SetText(XorStr("Hit optimazation"));
	ResolverOptions.SetSize(293, 252);
	ResolverOptions.SetPosition(333, 16);
	RegisterControl(&ResolverOptions);

	AntiAimCorrection.SetFileId(XorStr("r_antiaimcorrection"));
	ResolverOptions.PlaceLabeledControl(0, XorStr("Anti-aim correction"), this, &AntiAimCorrection);

	ResolverOverride.SetFileId(XorStr("r_resolveroverride"));
	ResolverOptions.PlaceLabeledControl(0, XorStr("Resolver override"), this, &ResolverOverride);

	ResolverOverrideKey.SetFileId(XorStr("r_resolveroverridekey"));
	ResolverOptions.PlaceLabeledControl(0, XorStr(""), this, &ResolverOverrideKey);

	DisableInterpolation.SetFileId(XorStr("r_disableinterp"));
	ResolverOptions.PlaceLabeledControl(0, XorStr("Disable interpolation"), this, &DisableInterpolation);

	Backtracking.SetFileId(XorStr("r_backtracking"));
	ResolverOptions.PlaceLabeledControl(0, XorStr("Backtracking"), this, &Backtracking);

	LinearExtrapolations.SetFileId(XorStr("r_linearextrapolation"));
	ResolverOptions.PlaceLabeledControl(0, XorStr("Linear Extrapolations"), this, &LinearExtrapolations);

	/*AngleLodge.SetFileId(XorStr("r_anglelodge"));
	ResolverOptions.PlaceLabeledControl(2, XorStr("Angle Lodge"), this, &AngleLodge);

	AngleLodgeSpeed.SetFileId(XorStr("r_anglelodgespeed"));
	AngleLodgeSpeed.SetFormat(SliderFormat::FORMAT_DECDIG2);
	AngleLodgeSpeed.SetValue(0.20);
	AngleLodgeSpeed.SetBoundaries(0, 1);
	ResolverOptions.PlaceLabeledControl(2, XorStr(""), this, &AngleLodgeSpeed);*/

	ResolverDebug.SetFileId(XorStr("r_resolverdebug"));
	ResolverOptions.PlaceLabeledControl(0, XorStr("Debug"), this, &ResolverDebug);

	LowerbodyIndicator.SetFileId(XorStr("r_lbyindicator"));
	ResolverOptions.PlaceLabeledControl(0, XorStr("Lowerbody indicator"), this, &LowerbodyIndicator);

	//Anti Aim
	DefaultAntiAimGroup.SetText(XorStr("Anti-Aim"));
	DefaultAntiAimGroup.SetSize(293, 272);
	DefaultAntiAimGroup.SetPosition(333, 284);
	DefaultAntiAimGroup.AddTab(CGroupTab("Stagnant", 1));
	DefaultAntiAimGroup.AddTab(CGroupTab("Moving", 2));
	DefaultAntiAimGroup.AddTab(CGroupTab("Edge", 3));
	DefaultAntiAimGroup.AddTab(CGroupTab("Modifiers", 4));
	RegisterControl(&DefaultAntiAimGroup);

	stagnant_aa.RealPitch.SetFileId(XorStr("r_stagnant_realpitch"));
	stagnant_aa.RealPitch.AddItem(XorStr("Off"));
	stagnant_aa.RealPitch.AddItem(XorStr("Down"));
	stagnant_aa.RealPitch.AddItem(XorStr("Fake down"));
	stagnant_aa.RealPitch.AddItem(XorStr("Up"));
	stagnant_aa.RealPitch.AddItem(XorStr("Fake up"));
	stagnant_aa.RealPitch.AddItem(XorStr("Random"));
	DefaultAntiAimGroup.PlaceLabeledControl(1, XorStr("Pitch"), this, &stagnant_aa.RealPitch);

	stagnant_aa.RealYaw.SetFileId(XorStr("r_stagnant_realyaw"));
	stagnant_aa.RealYaw.AddItem(XorStr("Off"));
	stagnant_aa.RealYaw.AddItem(XorStr("90°"));
	stagnant_aa.RealYaw.AddItem(XorStr("180°"));
	stagnant_aa.RealYaw.AddItem(XorStr("Crooked"));
	stagnant_aa.RealYaw.AddItem(XorStr("Jitter"));
	stagnant_aa.RealYaw.AddItem(XorStr("Swap"));
	stagnant_aa.RealYaw.AddItem(XorStr("Rotate"));
	stagnant_aa.RealYaw.AddItem(XorStr("Lowerbody"));
	stagnant_aa.RealYaw.AddItem(XorStr("Corruption"));
	DefaultAntiAimGroup.PlaceLabeledControl(1, XorStr("Real yaw"), this, &stagnant_aa.RealYaw);

	stagnant_aa.RealYawOffset.SetFileId(XorStr("r_stagnant_realyawoffset"));
	stagnant_aa.RealYawOffset.SetBoundaries(-180, 180);
	stagnant_aa.RealYawOffset.extension = XorStr("°");
	stagnant_aa.RealYawOffset.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(1, XorStr(""), this, &stagnant_aa.RealYawOffset);

	stagnant_aa.FakeYaw.SetFileId(XorStr("r_stagnant_fakeyaw"));
	stagnant_aa.FakeYaw.AddItem(XorStr("Off"));
	stagnant_aa.FakeYaw.AddItem(XorStr("90°"));
	stagnant_aa.FakeYaw.AddItem(XorStr("180°"));
	stagnant_aa.FakeYaw.AddItem(XorStr("Crooked"));
	stagnant_aa.FakeYaw.AddItem(XorStr("Jitter"));
	stagnant_aa.FakeYaw.AddItem(XorStr("Swap"));
	stagnant_aa.FakeYaw.AddItem(XorStr("Rotate"));
	stagnant_aa.FakeYaw.AddItem(XorStr("Lowerbody"));
	stagnant_aa.FakeYaw.AddItem(XorStr("Corruption"));
	DefaultAntiAimGroup.PlaceLabeledControl(1, XorStr("Fake yaw"), this, &stagnant_aa.FakeYaw);

	stagnant_aa.FakeYawOffset.SetFileId(XorStr("r_stagnant_fakeyawoffset"));
	stagnant_aa.FakeYawOffset.SetBoundaries(-180, 180);
	stagnant_aa.FakeYawOffset.extension = XorStr("°");
	stagnant_aa.FakeYawOffset.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(1, XorStr(""), this, &stagnant_aa.FakeYawOffset);

	stagnant_aa.CleanUp.SetFileId(XorStr("r_moving_cleanup"));
	DefaultAntiAimGroup.PlaceLabeledControl(1, XorStr("Clean up"), this, &stagnant_aa.CleanUp);

	moving_aa.RealPitch.SetFileId(XorStr("r_moving_realpitch"));
	moving_aa.RealPitch.AddItem(XorStr("Off"));
	moving_aa.RealPitch.AddItem(XorStr("Down"));
	moving_aa.RealPitch.AddItem(XorStr("Fake down"));
	moving_aa.RealPitch.AddItem(XorStr("Up"));
	moving_aa.RealPitch.AddItem(XorStr("Fake up"));
	moving_aa.RealPitch.AddItem(XorStr("Random"));
	DefaultAntiAimGroup.PlaceLabeledControl(2, XorStr("Pitch"), this, &moving_aa.RealPitch);

	moving_aa.RealYaw.SetFileId(XorStr("r_moving_realyaw"));
	moving_aa.RealYaw.AddItem(XorStr("Off"));
	moving_aa.RealYaw.AddItem(XorStr("90°"));
	moving_aa.RealYaw.AddItem(XorStr("180°"));
	moving_aa.RealYaw.AddItem(XorStr("Crooked"));
	moving_aa.RealYaw.AddItem(XorStr("Jitter"));
	moving_aa.RealYaw.AddItem(XorStr("Swap"));
	moving_aa.RealYaw.AddItem(XorStr("Rotate"));
	moving_aa.RealYaw.AddItem(XorStr("Lowerbody"));
	moving_aa.RealYaw.AddItem(XorStr("Corruption"));
	DefaultAntiAimGroup.PlaceLabeledControl(2, XorStr("Real yaw"), this, &moving_aa.RealYaw);

	moving_aa.RealYawOffset.SetFileId(XorStr("r_moving_realyawoffset"));
	moving_aa.RealYawOffset.SetBoundaries(-180, 180);
	moving_aa.RealYawOffset.extension = XorStr("°");
	moving_aa.RealYawOffset.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(2, XorStr(""), this, &moving_aa.RealYawOffset);

	moving_aa.FakeYaw.SetFileId(XorStr("r_moving_fakeyaw"));
	moving_aa.FakeYaw.AddItem(XorStr("Off"));
	moving_aa.FakeYaw.AddItem(XorStr("90°"));
	moving_aa.FakeYaw.AddItem(XorStr("180°"));
	moving_aa.FakeYaw.AddItem(XorStr("Crooked"));
	moving_aa.FakeYaw.AddItem(XorStr("Jitter"));
	moving_aa.FakeYaw.AddItem(XorStr("Swap"));
	moving_aa.FakeYaw.AddItem(XorStr("Rotate"));
	moving_aa.FakeYaw.AddItem(XorStr("Lowerbody"));
	moving_aa.FakeYaw.AddItem(XorStr("Corruption"));
	DefaultAntiAimGroup.PlaceLabeledControl(2, XorStr("Fake yaw"), this, &moving_aa.FakeYaw);

	moving_aa.FakeYawOffset.SetFileId(XorStr("r_moving_fakeyawoffset"));
	moving_aa.FakeYawOffset.SetBoundaries(-180, 180);
	moving_aa.FakeYawOffset.extension = XorStr("°");
	moving_aa.FakeYawOffset.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(2, XorStr(""), this, &moving_aa.FakeYawOffset);

	moving_aa.CleanUp.SetFileId(XorStr("r_moving_cleanup"));
	DefaultAntiAimGroup.PlaceLabeledControl(2, XorStr("Clean up"), this, &moving_aa.CleanUp);

	edge_aa.Type.SetFileId(XorStr("r_edge_walldtc"));
	edge_aa.Type.AddItem(XorStr("Off"));
	edge_aa.Type.AddItem(XorStr("Freestanding"));
	DefaultAntiAimGroup.PlaceLabeledControl(3, XorStr("Edge type"), this, &edge_aa.Type);

	edge_aa.RealPitch.SetFileId(XorStr("r_edge_realpitch"));
	edge_aa.RealPitch.AddItem(XorStr("Off"));
	edge_aa.RealPitch.AddItem(XorStr("Down"));
	edge_aa.RealPitch.AddItem(XorStr("Fake down"));
	edge_aa.RealPitch.AddItem(XorStr("Up"));
	edge_aa.RealPitch.AddItem(XorStr("Fake up"));
	edge_aa.RealPitch.AddItem(XorStr("Random"));
	DefaultAntiAimGroup.PlaceLabeledControl(3, XorStr("Pitch"), this, &edge_aa.RealPitch);

	edge_aa.RealYaw.SetFileId(XorStr("r_edge_realyaw"));
	edge_aa.RealYaw.AddItem(XorStr("Off"));
	edge_aa.RealYaw.AddItem(XorStr("90°"));
	edge_aa.RealYaw.AddItem(XorStr("180°"));
	edge_aa.RealYaw.AddItem(XorStr("Crooked"));
	edge_aa.RealYaw.AddItem(XorStr("Jitter"));
	edge_aa.RealYaw.AddItem(XorStr("Swap"));
	edge_aa.RealYaw.AddItem(XorStr("Rotate"));
	DefaultAntiAimGroup.PlaceLabeledControl(3, XorStr("Real yaw"), this, &edge_aa.RealYaw);

	edge_aa.RealYawOffset.SetFileId(XorStr("r_edge_realyawoffset"));
	edge_aa.RealYawOffset.SetBoundaries(-180, 180);
	edge_aa.RealYawOffset.extension = XorStr("°");
	edge_aa.RealYawOffset.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(3, XorStr(""), this, &edge_aa.RealYawOffset);

	edge_aa.FakeYaw.SetFileId(XorStr("r_edge_fakeyaw"));
	edge_aa.FakeYaw.AddItem(XorStr("Off"));
	edge_aa.FakeYaw.AddItem(XorStr("90°"));
	edge_aa.FakeYaw.AddItem(XorStr("180°"));
	edge_aa.FakeYaw.AddItem(XorStr("Crooked"));
	edge_aa.FakeYaw.AddItem(XorStr("Jitter"));
	edge_aa.FakeYaw.AddItem(XorStr("Swap"));
	edge_aa.FakeYaw.AddItem(XorStr("Rotate"));
	DefaultAntiAimGroup.PlaceLabeledControl(3, XorStr("Fake yaw"), this, &edge_aa.FakeYaw);

	edge_aa.FakeYawOffset.SetFileId(XorStr("r_edge_fakeyawoffset"));
	edge_aa.FakeYawOffset.SetBoundaries(-180, 180);
	edge_aa.FakeYawOffset.extension = XorStr("°");
	edge_aa.FakeYawOffset.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(3, XorStr(""), this, &edge_aa.FakeYawOffset);

	DormantCheck.SetFileId(XorStr("r_dormantcheck"));
	DefaultAntiAimGroup.PlaceLabeledControl(4, XorStr("Dormant check"), this, &DormantCheck);

	RealYawDirection.SetFileId(XorStr("r_realyawdirection"));
	RealYawDirection.AddItem(XorStr("Free look"));
	RealYawDirection.AddItem(XorStr("Static"));
	RealYawDirection.AddItem(XorStr("Dynamic"));
	DefaultAntiAimGroup.PlaceLabeledControl(4, XorStr("Yaw direction"), this, &RealYawDirection);

	RotateSpeed.SetFileId(XorStr("r_rotatespeed"));
	RotateSpeed.SetBoundaries(0, 5);
	RotateSpeed.SetValue(0);
	RotateSpeed.SetFormat(SliderFormat::FORMAT_DECDIG2);
	DefaultAntiAimGroup.PlaceLabeledControl(4, XorStr("Rotate speed"), this, &RotateSpeed);

	JitterRange.SetFileId(XorStr("r_jitterrange"));
	JitterRange.SetBoundaries(0, 180);
	JitterRange.extension = XorStr("°");
	JitterRange.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(4, XorStr("Jitter range"), this, &JitterRange);

	Lbybreaker.SetFileId("r_lbybreaker");
	DefaultAntiAimGroup.PlaceLabeledControl(4, XorStr("Lowerbody breaker"), this, &Lbybreaker);

	LbyDelta.SetFileId(XorStr("r_lbydelta"));
	LbyDelta.SetBoundaries(-180, 180);
	LbyDelta.extension = XorStr("°");
	LbyDelta.SetValue(0);
	DefaultAntiAimGroup.PlaceLabeledControl(4, XorStr(""), this, &LbyDelta);
}

void CLegitTab::Setup()
{
	SetTitle(XorStr("b"));

	AimbotGroup.SetText(XorStr("Aimbot"));
	AimbotGroup.SetSize(293, 540);
	AimbotGroup.SetPosition(24, 16);
	AimbotGroup.AddTab(CGroupTab("Pistol", 1));
	AimbotGroup.AddTab(CGroupTab("SMG", 2));
	AimbotGroup.AddTab(CGroupTab("Rifle", 3));
	AimbotGroup.AddTab(CGroupTab("Shotgun", 4));
	AimbotGroup.AddTab(CGroupTab("Snipers", 5));
	RegisterControl(&AimbotGroup);

	// Pistol
	pistol.Enabled.SetFileId(XorStr("pistol_enabled"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Activate"), this, &pistol.Enabled);

	pistol.AimbotMode.SetFileId(XorStr("pistol_mode"));
	pistol.AimbotMode.AddItem(XorStr("Visualized"));
	pistol.AimbotMode.AddItem(XorStr("Non-visualized"));
	pistol.AimbotMode.AddItem(XorStr("Anti-aim"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Aimbot mode"), this, &pistol.AimbotMode);

	pistol.AutoFire.SetFileId(XorStr("pistol_autofire"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Auto fire"), this, &pistol.AutoFire);

	pistol.AutoFireKey.SetFileId(XorStr("pistol_autofirekey"));
	AimbotGroup.PlaceLabeledControl(1, XorStr(""), this, &pistol.AutoFireKey);

	pistol.AutoFireMode.SetFileId(XorStr("pistol_autofiremode"));
	pistol.AutoFireMode.AddItem(XorStr("Bone"));
	pistol.AutoFireMode.AddItem(XorStr("Key Press"));
	AimbotGroup.PlaceLabeledControl(1, XorStr(""), this, &pistol.AutoFireMode);

	pistol.AutoFireTarget.SetFileId(XorStr("pistol_autofiretarget"));
	pistol.AutoFireTarget.AddItem(XorStr("Head"));
	pistol.AutoFireTarget.AddItem(XorStr("Neck"));
	pistol.AutoFireTarget.AddItem(XorStr("Chest"));
	pistol.AutoFireTarget.AddItem(XorStr("Stomach"));
	pistol.AutoFireTarget.AddItem(XorStr("Pelvis"));
	pistol.AutoFireTarget.AddItem(XorStr("Left arm"));
	pistol.AutoFireTarget.AddItem(XorStr("Right arm"));
	pistol.AutoFireTarget.AddItem(XorStr("Left leg"));
	pistol.AutoFireTarget.AddItem(XorStr("Right leg"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Autofire bone"), this, &pistol.AutoFireTarget);

	pistol.FieldOfView.SetFileId(XorStr("pistol_fov"));
	pistol.FieldOfView.SetBoundaries(0, 90);
	pistol.FieldOfView.extension = XorStr("°");
	pistol.FieldOfView.SetValue(0);
	pistol.FieldOfView.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(1, XorStr("Limit FOV"), this, &pistol.FieldOfView);

	pistol.ReactionTime.SetFileId(XorStr("pistol_reaction"));
	pistol.ReactionTime.AddItem(XorStr("Slow"));
	pistol.ReactionTime.AddItem(XorStr("Medium"));
	pistol.ReactionTime.AddItem(XorStr("Fast"));
	pistol.ReactionTime.AddItem(XorStr("Insane"));
	AimbotGroup.PlaceLabeledControl(1, XorStr("Reaction time"), this, &pistol.ReactionTime);

	pistol.Smoothing.SetFileId(XorStr("pistol_smoothing"));
	pistol.Smoothing.SetBoundaries(0, 5);
	pistol.Smoothing.SetValue(0);
	pistol.Smoothing.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(1, XorStr("Sensitivity"), this, &pistol.Smoothing);

	pistol.Recoil.SetFileId(XorStr("pistol_recoil"));
	pistol.Recoil.SetBoundaries(0, 2);
	pistol.Recoil.SetValue(0);
	pistol.Recoil.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(1, XorStr("Recoil"), this, &pistol.Recoil);

	// SMG
	smg.Enabled.SetFileId(XorStr("smg_enabled"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Activate"), this, &smg.Enabled);

	smg.AimbotMode.SetFileId(XorStr("smg_mode"));
	smg.AimbotMode.AddItem(XorStr("Visualized"));
	smg.AimbotMode.AddItem(XorStr("Non-visualized"));
	smg.AimbotMode.AddItem(XorStr("Anti-aim"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Aimbot mode"), this, &smg.AimbotMode);

	smg.AutoFire.SetFileId(XorStr("smg_autofire"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Auto fire"), this, &smg.AutoFire);

	smg.AutoFireKey.SetFileId(XorStr("smg_autofirekey"));
	AimbotGroup.PlaceLabeledControl(2, XorStr(""), this, &smg.AutoFireKey);

	smg.AutoFireMode.SetFileId(XorStr("smg_autofiremode"));
	smg.AutoFireMode.AddItem(XorStr("Bone"));
	smg.AutoFireMode.AddItem(XorStr("Key Press"));
	AimbotGroup.PlaceLabeledControl(2, XorStr(""), this, &smg.AutoFireMode);

	smg.AutoFireTarget.SetFileId(XorStr("smg_autofiretarget"));
	smg.AutoFireTarget.AddItem(XorStr("Head"));
	smg.AutoFireTarget.AddItem(XorStr("Neck"));
	smg.AutoFireTarget.AddItem(XorStr("Chest"));
	smg.AutoFireTarget.AddItem(XorStr("Stomach"));
	smg.AutoFireTarget.AddItem(XorStr("Pelvis"));
	smg.AutoFireTarget.AddItem(XorStr("Left arm"));
	smg.AutoFireTarget.AddItem(XorStr("Right arm"));
	smg.AutoFireTarget.AddItem(XorStr("Left leg"));
	smg.AutoFireTarget.AddItem(XorStr("Right leg"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Autofire bone"), this, &smg.AutoFireTarget);

	smg.FieldOfView.SetFileId(XorStr("smg_fov"));
	smg.FieldOfView.SetBoundaries(0, 90);
	smg.FieldOfView.extension = XorStr("°");
	smg.FieldOfView.SetValue(0);
	smg.FieldOfView.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(2, XorStr("Limit FOV"), this, &smg.FieldOfView);

	smg.ReactionTime.SetFileId(XorStr("smg_reaction"));
	smg.ReactionTime.AddItem(XorStr("Slow"));
	smg.ReactionTime.AddItem(XorStr("Medium"));
	smg.ReactionTime.AddItem(XorStr("Fast"));
	smg.ReactionTime.AddItem(XorStr("Insane"));
	AimbotGroup.PlaceLabeledControl(2, XorStr("Reaction time"), this, &smg.ReactionTime);

	smg.Smoothing.SetFileId(XorStr("smg_smoothing"));
	smg.Smoothing.SetBoundaries(0, 5);
	smg.Smoothing.SetValue(0);
	smg.Smoothing.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(2, XorStr("Sensitivity"), this, &smg.Smoothing);

	smg.Recoil.SetFileId(XorStr("smg_recoil"));
	smg.Recoil.SetBoundaries(0, 2);
	smg.Recoil.SetValue(0);
	smg.Recoil.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(2, XorStr("Recoil"), this, &smg.Recoil);

	// Rifle
	rifle.Enabled.SetFileId(XorStr("rifle_enabled"));
	AimbotGroup.PlaceLabeledControl(3, XorStr("Activate"), this, &rifle.Enabled);

	rifle.AimbotMode.SetFileId(XorStr("rifle_mode"));
	rifle.AimbotMode.AddItem(XorStr("Visualized"));
	rifle.AimbotMode.AddItem(XorStr("Non-visualized"));
	rifle.AimbotMode.AddItem(XorStr("Anti-aim"));
	AimbotGroup.PlaceLabeledControl(3, XorStr("Aimbot mode"), this, &rifle.AimbotMode);

	rifle.AutoFire.SetFileId(XorStr("rifle_autofire"));
	AimbotGroup.PlaceLabeledControl(3, XorStr("Auto fire"), this, &rifle.AutoFire);

	rifle.AutoFireKey.SetFileId(XorStr("rifle_autofirekey"));
	AimbotGroup.PlaceLabeledControl(3, XorStr(""), this, &rifle.AutoFireKey);

	rifle.AutoFireMode.SetFileId(XorStr("rifle_autofiremode"));
	rifle.AutoFireMode.AddItem(XorStr("Bone"));
	rifle.AutoFireMode.AddItem(XorStr("Key Press"));
	AimbotGroup.PlaceLabeledControl(3, XorStr(""), this, &rifle.AutoFireMode);

	rifle.AutoFireTarget.SetFileId(XorStr("rifle_autofiretarget"));
	rifle.AutoFireTarget.AddItem(XorStr("Head"));
	rifle.AutoFireTarget.AddItem(XorStr("Neck"));
	rifle.AutoFireTarget.AddItem(XorStr("Chest"));
	rifle.AutoFireTarget.AddItem(XorStr("Stomach"));
	rifle.AutoFireTarget.AddItem(XorStr("Pelvis"));
	rifle.AutoFireTarget.AddItem(XorStr("Left arm"));
	rifle.AutoFireTarget.AddItem(XorStr("Right arm"));
	rifle.AutoFireTarget.AddItem(XorStr("Left leg"));
	rifle.AutoFireTarget.AddItem(XorStr("Right leg"));
	AimbotGroup.PlaceLabeledControl(3, XorStr("Autofire bone"), this, &rifle.AutoFireTarget);

	rifle.FieldOfView.SetFileId(XorStr("rifle_fov"));
	rifle.FieldOfView.SetBoundaries(0, 90);
	rifle.FieldOfView.extension = XorStr("°");
	rifle.FieldOfView.SetValue(0);
	rifle.FieldOfView.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(3, XorStr("Limit FOV"), this, &rifle.FieldOfView);

	rifle.ReactionTime.SetFileId(XorStr("rifle_reaction"));
	rifle.ReactionTime.AddItem(XorStr("Slow"));
	rifle.ReactionTime.AddItem(XorStr("Medium"));
	rifle.ReactionTime.AddItem(XorStr("Fast"));
	rifle.ReactionTime.AddItem(XorStr("Insane"));
	AimbotGroup.PlaceLabeledControl(3, XorStr("Reaction time"), this, &rifle.ReactionTime);

	rifle.Smoothing.SetFileId(XorStr("rifle_smoothing"));
	rifle.Smoothing.SetBoundaries(0, 5);
	rifle.Smoothing.SetValue(0);
	rifle.Smoothing.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(3, XorStr("Sensitivity"), this, &rifle.Smoothing);

	rifle.Recoil.SetFileId(XorStr("rifle_recoil"));
	rifle.Recoil.SetBoundaries(0, 2);
	rifle.Recoil.SetValue(0);
	rifle.Recoil.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(3, XorStr("Recoil"), this, &rifle.Recoil);

	// Shotgun
	shotgun.Enabled.SetFileId(XorStr("shotgun_enabled"));
	AimbotGroup.PlaceLabeledControl(4, XorStr("Activate"), this, &shotgun.Enabled);

	shotgun.AimbotMode.SetFileId(XorStr("shotgun_mode"));
	shotgun.AimbotMode.AddItem(XorStr("Visualized"));
	shotgun.AimbotMode.AddItem(XorStr("Non-visualized"));
	shotgun.AimbotMode.AddItem(XorStr("Anti-aim"));
	AimbotGroup.PlaceLabeledControl(4, XorStr("Aimbot mode"), this, &shotgun.AimbotMode);

	shotgun.AutoFire.SetFileId(XorStr("shotgun_autofire"));
	AimbotGroup.PlaceLabeledControl(4, XorStr("Auto fire"), this, &shotgun.AutoFire);

	shotgun.AutoFireKey.SetFileId(XorStr("shotgun_autofirekey"));
	AimbotGroup.PlaceLabeledControl(4, XorStr(""), this, &shotgun.AutoFireKey);

	shotgun.AutoFireMode.SetFileId(XorStr("shotgun_autofiremode"));
	shotgun.AutoFireMode.AddItem(XorStr("Bone"));
	shotgun.AutoFireMode.AddItem(XorStr("Key Press"));
	AimbotGroup.PlaceLabeledControl(4, XorStr(""), this, &shotgun.AutoFireMode);

	shotgun.AutoFireTarget.SetFileId(XorStr("shotgun_autofiretarget"));
	shotgun.AutoFireTarget.AddItem(XorStr("Head"));
	shotgun.AutoFireTarget.AddItem(XorStr("Neck"));
	shotgun.AutoFireTarget.AddItem(XorStr("Chest"));
	shotgun.AutoFireTarget.AddItem(XorStr("Stomach"));
	shotgun.AutoFireTarget.AddItem(XorStr("Pelvis"));
	shotgun.AutoFireTarget.AddItem(XorStr("Left arm"));
	shotgun.AutoFireTarget.AddItem(XorStr("Right arm"));
	shotgun.AutoFireTarget.AddItem(XorStr("Left leg"));
	shotgun.AutoFireTarget.AddItem(XorStr("Right leg"));
	AimbotGroup.PlaceLabeledControl(4, XorStr("Autofire bone"), this, &shotgun.AutoFireTarget);

	shotgun.FieldOfView.SetFileId(XorStr("shotgun_fov"));
	shotgun.FieldOfView.SetBoundaries(0, 90);
	shotgun.FieldOfView.extension = XorStr("°");
	shotgun.FieldOfView.SetValue(0);
	shotgun.FieldOfView.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(4, XorStr("Limit FOV"), this, &shotgun.FieldOfView);

	shotgun.ReactionTime.SetFileId(XorStr("shotgun_reaction"));
	shotgun.ReactionTime.AddItem(XorStr("Slow"));
	shotgun.ReactionTime.AddItem(XorStr("Medium"));
	shotgun.ReactionTime.AddItem(XorStr("Fast"));
	shotgun.ReactionTime.AddItem(XorStr("Insane"));
	AimbotGroup.PlaceLabeledControl(4, XorStr("Reaction time"), this, &shotgun.ReactionTime);

	shotgun.Smoothing.SetFileId(XorStr("shotgun_smoothing"));
	shotgun.Smoothing.SetBoundaries(0, 5);
	shotgun.Smoothing.SetValue(0);
	shotgun.Smoothing.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(4, XorStr("Sensitivity"), this, &shotgun.Smoothing);

	shotgun.Recoil.SetFileId(XorStr("shotgun_recoil"));
	shotgun.Recoil.SetBoundaries(0, 2);
	shotgun.Recoil.SetValue(0);
	shotgun.Recoil.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(4, XorStr("Recoil"), this, &shotgun.Recoil);

	// Sniper
	sniper.Enabled.SetFileId(XorStr("sniper_enabled"));
	AimbotGroup.PlaceLabeledControl(5, XorStr("Activate"), this, &sniper.Enabled);

	sniper.AimbotMode.SetFileId(XorStr("sniper_mode"));
	sniper.AimbotMode.AddItem(XorStr("Visualized"));
	sniper.AimbotMode.AddItem(XorStr("Non-visualized"));
	sniper.AimbotMode.AddItem(XorStr("Anti-aim"));
	AimbotGroup.PlaceLabeledControl(5, XorStr("Aimbot mode"), this, &sniper.AimbotMode);

	sniper.AutoFire.SetFileId(XorStr("sniper_autofire"));
	AimbotGroup.PlaceLabeledControl(5, XorStr("Auto fire"), this, &sniper.AutoFire);

	sniper.AutoFireKey.SetFileId(XorStr("sniper_autofirekey"));
	AimbotGroup.PlaceLabeledControl(5, XorStr(""), this, &sniper.AutoFireKey);

	sniper.AutoFireMode.SetFileId(XorStr("sniper_autofiremode"));
	sniper.AutoFireMode.AddItem(XorStr("Bone"));
	sniper.AutoFireMode.AddItem(XorStr("Key Press"));
	AimbotGroup.PlaceLabeledControl(5, XorStr(""), this, &sniper.AutoFireMode);

	sniper.AutoFireTarget.SetFileId(XorStr("sniper_autofiretarget"));
	sniper.AutoFireTarget.AddItem(XorStr("Head"));
	sniper.AutoFireTarget.AddItem(XorStr("Neck"));
	sniper.AutoFireTarget.AddItem(XorStr("Chest"));
	sniper.AutoFireTarget.AddItem(XorStr("Stomach"));
	sniper.AutoFireTarget.AddItem(XorStr("Pelvis"));
	sniper.AutoFireTarget.AddItem(XorStr("Left arm"));
	sniper.AutoFireTarget.AddItem(XorStr("Right arm"));
	sniper.AutoFireTarget.AddItem(XorStr("Left leg"));
	sniper.AutoFireTarget.AddItem(XorStr("Right leg"));
	AimbotGroup.PlaceLabeledControl(5, XorStr("Autofire bone"), this, &sniper.AutoFireTarget);

	sniper.FieldOfView.SetFileId(XorStr("sniper_fov"));
	sniper.FieldOfView.SetBoundaries(0, 90);
	sniper.FieldOfView.extension = XorStr("°");
	sniper.FieldOfView.SetValue(0);
	sniper.FieldOfView.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(5, XorStr("Limit FOV"), this, &sniper.FieldOfView);

	sniper.ReactionTime.SetFileId(XorStr("sniper_reaction"));
	sniper.ReactionTime.AddItem(XorStr("Slow"));
	sniper.ReactionTime.AddItem(XorStr("Medium"));
	sniper.ReactionTime.AddItem(XorStr("Fast"));
	sniper.ReactionTime.AddItem(XorStr("Insane"));
	AimbotGroup.PlaceLabeledControl(5, XorStr("Reaction time"), this, &sniper.ReactionTime);

	sniper.Smoothing.SetFileId(XorStr("sniper_smoothing"));
	sniper.Smoothing.SetBoundaries(0, 5);
	sniper.Smoothing.SetValue(0);
	sniper.Smoothing.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(5, XorStr("Sensitivity"), this, &sniper.Smoothing);

	sniper.Recoil.SetFileId(XorStr("sniper_recoil"));
	sniper.Recoil.SetBoundaries(0, 2);
	sniper.Recoil.SetValue(0);
	sniper.Recoil.SetFormat(FORMAT_DECDIG2);
	AimbotGroup.PlaceLabeledControl(5, XorStr("Recoil"), this, &sniper.Recoil);
}

void CVisualTab::Setup()
{
	SetTitle(XorStr("c"));

	// Player ESP
	PlayerESP.SetText(XorStr("Player ESP"));
	PlayerESP.SetSize(293, 380);
	PlayerESP.SetPosition(24, 16);
	PlayerESP.AddTab(CGroupTab(XorStr("Main"), 1));
	PlayerESP.AddTab(CGroupTab(XorStr("Other"), 2));
	RegisterControl(&PlayerESP);

	player.ActivationType.SetFileId(XorStr("v_player_activationtype"));
	player.ActivationType.AddItem(XorStr("Default"));
	player.ActivationType.AddItem(XorStr("Visible Only"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Activation type"), this, &player.ActivationType);

	player.EspTeammates.SetFileId(XorStr("v_player_teammates"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Teammates"), this, &player.EspTeammates);

	player.EspBox.SetFileId(XorStr("v_player_espbox"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Box"), this, &player.EspBox);

	player.BoxColor.SetFileId(XorStr("v_player_espbox_color"));
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.BoxColor);

	player.BoxType.SetFileId(XorStr("v_player_boxtype"));
	player.BoxType.AddItem(XorStr("Full"));
	player.BoxType.AddItem(XorStr("Cornered"));
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.BoxType);

	player.EspMode.SetFileId(XorStr("v_player_espmode"));
	player.EspMode.AddItem(XorStr("Static"));
	player.EspMode.AddItem(XorStr("Dynamic"));
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.EspMode);

	player.EspOutlines.SetFileId(XorStr("v_player_outlines"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Outlined"), this, &player.EspOutlines);

	player.BoxFill.SetFileId(XorStr("v_player_boxfill"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Box Fill"), this, &player.BoxFill);

	player.BoxFillOpacity.SetFileId(XorStr("v_player_boxfillopacity"));
	player.BoxFillOpacity.SetBoundaries(0, 100);
	player.BoxFillOpacity.extension = XorStr("%%");
	player.BoxFillOpacity.SetValue(0);
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.BoxFillOpacity);

	player.Glow.SetFileId(XorStr("v_player_glow"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Glow"), this, &player.Glow);

	player.GlowColor.SetFileId(XorStr("v_player_glowcolor"));
	player.GlowColor.SetColor(24, 130, 230, 255);
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.GlowColor);

	player.GlowOpacity.SetFileId(XorStr("v_player_glowopacity"));
	player.GlowOpacity.SetBoundaries(0, 100);
	player.GlowOpacity.extension = XorStr("%%");
	player.GlowOpacity.SetValue(0);
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.GlowOpacity);

	player.ShowHealth.SetFileId(XorStr("v_player_health"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Health"), this, &player.ShowHealth);

	player.ShowHealthText.SetFileId(XorStr("v_player_healthtext"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Health text"), this, &player.ShowHealthText);

	player.ShowArmor.SetFileId(XorStr("v_player_armor"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Armor"), this, &player.ShowArmor);

	player.ArmorColor.SetFileId(XorStr("v_player_armor_color"));
	player.ArmorColor.SetColor(86, 123, 190, 255);
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.ArmorColor);

	player.ShowSkeleton.SetFileId(XorStr("v_player_skeleton"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Skeleton"), this, &player.ShowSkeleton);

	player.SkeletonColor.SetFileId(XorStr("v_player_skeleton_color"));
	player.SkeletonColor.SetColor(255, 255, 255, 255);
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.SkeletonColor);

	player.ShowHitbones.SetFileId(XorStr("v_player_hitbones"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Hitbones"), this, &player.ShowHitbones);

	player.HitbonesColor.SetFileId(XorStr("v_player_hitbones_color"));
	player.HitbonesColor.SetColor(255, 255, 255, 255);
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.HitbonesColor);

	player.SnapLines.SetFileId(XorStr("v_player_snaplines"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Snap lines"), this, &player.SnapLines);

	player.SnapLinesColor.SetFileId(XorStr("v_player_snaplines_color"));
	player.SnapLinesColor.SetColor(255, 255, 255, 255);
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.SnapLinesColor);

	player.DirectionArrow.SetFileId(XorStr("v_direction_arrow"));
	PlayerESP.PlaceLabeledControl(1, XorStr("Direction arrow"), this, &player.DirectionArrow);

	player.DirectionArrowColor.SetFileId(XorStr("v_direction_arrowcolor"));
	PlayerESP.PlaceLabeledControl(1, XorStr(""), this, &player.DirectionArrowColor);

	player.ShowPlayerNames.SetFileId(XorStr("v_player_playernames"));
	PlayerESP.PlaceLabeledControl(2, XorStr("Player name"), this, &player.ShowPlayerNames);

	player.ShowWeaponDisplay.SetFileId(XorStr("v_player_weapondisplay"));
	PlayerESP.PlaceLabeledControl(2, XorStr("Weapon display"), this, &player.ShowWeaponDisplay);

	player.ShowWeaponDisplayType.SetFileId(XorStr("v_player_weapondisplaymode"));
	player.ShowWeaponDisplayType.AddItem(XorStr("Default"));
	player.ShowWeaponDisplayType.AddItem(XorStr("Name"));
	player.ShowWeaponDisplayType.AddItem(XorStr("Icons"));
	player.ShowWeaponDisplayType.AddItem(XorStr("Ammo"));
	PlayerESP.PlaceLabeledControl(2, XorStr("Mode"), this, &player.ShowWeaponDisplayType);

	player.HitAngle.SetFileId(XorStr("v_player_hitangle"));
	PlayerESP.PlaceLabeledControl(2, XorStr("Hit angle"), this, &player.HitAngle);

	player.Scoped.SetFileId(XorStr("v_player_scoped"));
	PlayerESP.PlaceLabeledControl(2, XorStr("Scoped"), this, &player.Scoped);

	player.Defusing.SetFileId(XorStr("v_player_defusing"));
	PlayerESP.PlaceLabeledControl(2, XorStr("Defusing"), this, &player.Defusing);

	player.InAir.SetFileId(XorStr("v_player_inair"));
	PlayerESP.PlaceLabeledControl(2, XorStr("In air"), this, &player.InAir);

	// Chams Group
	ChamsGroup.SetText(XorStr("Colored models"));
	ChamsGroup.SetSize(293, 144);
	ChamsGroup.SetPosition(24, 412);
	RegisterControl(&ChamsGroup);

	PlayerChamType.SetFileId(XorStr("v_coloredmodels_mode"));
	PlayerChamType.AddItem(XorStr("Default"));
	PlayerChamType.AddItem(XorStr("Flat"));
	PlayerChamType.AddItem(XorStr("Wireframe"));
	PlayerChamType.AddItem(XorStr("Glass"));
	ChamsGroup.PlaceLabeledControl(0, XorStr("Mode"), this, &PlayerChamType);

	ChamsEnemies.SetFileId(XorStr("v_coloredmodels_enemies"));
	ChamsGroup.PlaceLabeledControl(0, XorStr("Enemies"), this, &ChamsEnemies);

	ChamsEnemies_Color.SetFileId(XorStr("v_coloredmodels_enemies_color"));
	ChamsGroup.PlaceLabeledControl(0, XorStr(""), this, &ChamsEnemies_Color);

	ChamsEnemiesBehindWall.SetFileId(XorStr("v_coloredmodels_enemiesnovis"));
	ChamsGroup.PlaceLabeledControl(0, XorStr("Enemies (behind wall)"), this, &ChamsEnemiesBehindWall);

	ChamsEnemiesBehindWall_Color.SetFileId(XorStr("v_coloredmodels_enemiesnovis_color"));
	ChamsGroup.PlaceLabeledControl(0, XorStr(""), this, &ChamsEnemiesBehindWall_Color);

	ChamsTeammates.SetFileId(XorStr("v_coloredmodels_teammates"));
	ChamsGroup.PlaceLabeledControl(0, XorStr("Teammates"), this, &ChamsTeammates);

	ChamsTeammates_Color.SetFileId(XorStr("v_coloredmodels_teammates_color"));
	ChamsGroup.PlaceLabeledControl(0, XorStr(""), this, &ChamsTeammates_Color);

	ChamsTeammatesBehindWall.SetFileId(XorStr("v_coloredmodels_teammatesnovis"));
	ChamsGroup.PlaceLabeledControl(0, XorStr("Teammates (behind wall)"), this, &ChamsTeammatesBehindWall);

	ChamsTeammatesBehindWall_Color.SetFileId(XorStr("v_coloredmodels_teammatesnovis_color"));
	ChamsGroup.PlaceLabeledControl(0, XorStr(""), this, &ChamsTeammatesBehindWall_Color);

	// Other ESP
	OtherESP.SetText(XorStr("Other ESP"));
	OtherESP.SetSize(293, 150);
	OtherESP.SetPosition(333, 16);
	RegisterControl(&OtherESP);

	FilterWeapons.SetFileId(XorStr("v_filter_weapons"));
	OtherESP.PlaceLabeledControl(0, XorStr("Weapons"), this, &FilterWeapons);

	WeaponsColor.SetFileId(XorStr("v_weapons_color"));
	OtherESP.PlaceLabeledControl(0, XorStr(""), this, &WeaponsColor);

	FilterNades.SetFileId("v_filter_nades");
	OtherESP.PlaceLabeledControl(0, XorStr("Grenades"), this, &FilterNades);

	NadesColor.SetFileId(XorStr("v_nades_color"));
	OtherESP.PlaceLabeledControl(0, XorStr(""), this, &NadesColor);

	FilterBomb.SetFileId(XorStr("v_filter_bomb"));
	OtherESP.PlaceLabeledControl(0, XorStr("Bomb"), this, &FilterBomb);

	BombColor.SetFileId(XorStr("v_bomb_color"));
	OtherESP.PlaceLabeledControl(0, XorStr(""), this, &BombColor);

	Hitmarkers.SetFileId(XorStr("v_hitmarkers"));
	OtherESP.PlaceLabeledControl(0, XorStr("Hitmarkers"), this, &Hitmarkers);

	DamageIndicators.SetFileId(XorStr("v_damageindicators"));
	OtherESP.PlaceLabeledControl(0, XorStr("Damage indicators"), this, &DamageIndicators);

	SpreadCrosshair.SetFileId(XorStr("v_spreadcrosshair"));
	OtherESP.PlaceLabeledControl(0, XorStr("Spread crosshair"), this, &SpreadCrosshair);

	// PenetrationReticle.SetFileId(XorStr("v_penetrationreticle"));
	// OtherESP.PlaceLabeledControl(0, XorStr("Penetration reticle"), this, &PenetrationReticle);

	AntiAimLines.SetFileId("v_aalines");
	OtherESP.PlaceLabeledControl(0, XorStr("Anti-aim lines"), this, &AntiAimLines);

	GhostChams.SetFileId("v_ghostchams");
	OtherESP.PlaceLabeledControl(0, XorStr("Fake angles chams"), this, &GhostChams);

	// Effects
	Effects.SetText(XorStr("Effects"));
	Effects.SetSize(293, 283);
	Effects.SetPosition(333, 182);
	RegisterControl(&Effects);

	FovChanger.SetFileId(XorStr("v_fieldofview"));
	FovChanger.SetBoundaries(0, 90);
	FovChanger.SetValue(0);
	Effects.PlaceLabeledControl(0, XorStr("Field of view"), this, &FovChanger);

	RemoveParticles.SetFileId(XorStr("v_removeparticles"));
	Effects.PlaceLabeledControl(0, XorStr("Remove particles"), this, &RemoveParticles);

	RemoveSmoke.SetFileId(XorStr("v_removesmoke"));
	Effects.PlaceLabeledControl(0, XorStr("Remove smoke"), this, &RemoveSmoke);

	RemoveFlash.SetFileId(XorStr("v_removesmoke"));
	Effects.PlaceLabeledControl(0, XorStr("Remove flash"), this, &RemoveFlash);

	NoVisualRecoil.SetFileId(XorStr("v_novisualrecoil"));
	Effects.PlaceLabeledControl(0, XorStr("Remove visual recoil"), this, &NoVisualRecoil);

	RemoveWeaponScope.SetFileId(XorStr("v_removeweaponscope"));
	Effects.PlaceLabeledControl(0, XorStr("Remove scope"), this, &RemoveWeaponScope);

	Thirdperson.SetFileId(XorStr("v_thirdperson"));
	Effects.PlaceLabeledControl(0, XorStr("Force third person"), this, &Thirdperson);

	ThirdpersonToggle.SetFileId(XorStr("v_thirdpersontoggle"));
	Effects.PlaceLabeledControl(0, XorStr(""), this, &ThirdpersonToggle);

	VisualizedAngle.SetFileId(XorStr("v_visualizedangle"));
	VisualizedAngle.AddItem(XorStr("Real"));
	VisualizedAngle.AddItem(XorStr("Fake"));
	VisualizedAngle.AddItem(XorStr("Lowerbody"));
	Effects.PlaceLabeledControl(0, XorStr(""), this, &VisualizedAngle);

	NightMode.SetFileId(XorStr("v_nightmode"));
	Effects.PlaceLabeledControl(0, XorStr("Nightmode"), this, &NightMode);

	DisablePostProcessing.SetFileId(XorStr("v_disablepostprocessing"));
	Effects.PlaceLabeledControl(0, XorStr("Disable post-processing"), this, &DisablePostProcessing);

	MinecraftMode.SetFileId(XorStr("v_minecraftmode"));
	Effects.PlaceLabeledControl(0, XorStr("Minecraft mode"), this, &MinecraftMode);

	SkyboxChanger.SetFileId(XorStr("v_skyboxchanger"));
	Effects.PlaceLabeledControl(0, XorStr("Skybox changer"), this, &SkyboxChanger);

	SkyboxModel.SetFileId(XorStr("v_coloredmodels_mode"));
	SkyboxModel.AddItem(XorStr("Night time"));
	SkyboxModel.AddItem(XorStr("Pure black"));
	SkyboxModel.AddItem(XorStr("Mountains"));
	Effects.PlaceLabeledControl(0, XorStr("Skybox"), this, &SkyboxModel);
}

void CMiscTab::Setup()
{
	SetTitle(XorStr("d"));

	//Miscellaneous
	Miscellaneous.SetText(XorStr("Miscellaneous"));
	Miscellaneous.SetSize(293, 540);
	Miscellaneous.SetPosition(24, 16);
	RegisterControl(&Miscellaneous);

	BunnyHop.SetFileId(XorStr("m_autohop"));
	Miscellaneous.PlaceLabeledControl(0, XorStr("Bunnyhop"), this, &BunnyHop);

	Autostrafe.SetFileId(XorStr("m_autostrafe"));
	Miscellaneous.PlaceLabeledControl(0, XorStr("Air strafe"), this, &Autostrafe);

	AutoStrafeType.SetFileId(XorStr("m_autostrafetype"));
	AutoStrafeType.AddItem(XorStr("Normal"));
	AutoStrafeType.AddItem(XorStr("Silent"));
	Miscellaneous.PlaceLabeledControl(0, XorStr(""), this, &AutoStrafeType);

	FakeWalk.SetFileId(XorStr("m_fakewalk"));
	Miscellaneous.PlaceLabeledControl(0, XorStr("Fake walk"), this, &FakeWalk);

	FakeWalkKey.SetFileId(XorStr("m_fakewalkkey"));
	Miscellaneous.PlaceLabeledControl(0, XorStr(""), this, &FakeWalkKey);

	Blockbot.SetFileId("m_blockbot");
	Miscellaneous.PlaceLabeledControl(0, "Blockbot", this, &Blockbot);

	BlockbotBind.SetFileId("m_blockbotkey");
	Miscellaneous.PlaceLabeledControl(0, "", this, &BlockbotBind);

	AutoAccept.SetFileId(XorStr("m_autoaccept"));
	Miscellaneous.PlaceLabeledControl(0, XorStr("Auto-accept"), this, &AutoAccept);

	ClanChanger.SetFileId(XorStr("m_clanchanger"));
	Miscellaneous.PlaceLabeledControl(0, XorStr("Clantag changer"), this, &ClanChanger);

	NameChanger.SetFileId(XorStr("m_namechanger"));
	Miscellaneous.PlaceLabeledControl(0, XorStr("Name changer"), this, &NameChanger);

	NameChangerType.SetFileId(XorStr("m_namechangertype"));
	NameChangerType.AddItem(XorStr("Off"));
	NameChangerType.AddItem(XorStr("Ayyware crasher"));
	NameChangerType.AddItem(XorStr("Name stealer"));
	NameChangerType.AddItem(XorStr("Invisible"));
	Miscellaneous.PlaceLabeledControl(0, XorStr(""), this, &NameChangerType);

	//Settings
	SettingsGroup.SetText(XorStr("Settings"));
	SettingsGroup.SetSize(293, 90);
	SettingsGroup.SetPosition(333, 16);
	RegisterControl(&SettingsGroup);

	MenuKeyLabel.SetText(XorStr("Menu key"));
	SettingsGroup.PlaceLabeledControl(0, XorStr(""), this, &MenuKeyLabel);

	MenuKey.SetFileId(XorStr("m_menukey"));
	MenuKey.SetKey(VK_INSERT);
	SettingsGroup.PlaceLabeledControl(0, XorStr(""), this, &MenuKey);

	MenuColor.SetFileId(XorStr("m_menucolor"));
	MenuColor.SetColor(24, 130, 230, 255);
	SettingsGroup.PlaceLabeledControl(0, XorStr("Menu color"), this, &MenuColor);

	CheckboxUncheckedColor.SetFileId(XorStr("m_checkboxunchecked"));
	CheckboxUncheckedColor.SetColor(255, 255, 255, 255);
	SettingsGroup.PlaceLabeledControl(0, XorStr("Checkbox unchecked"), this, &CheckboxUncheckedColor);

	AntiUntrusted.SetFileId(XorStr("m_antiuntrusted"));
	AntiUntrusted.SetState(true);
	SettingsGroup.PlaceLabeledControl(0, XorStr("Anti-unstrusted"), this, &AntiUntrusted);

	//Flag
	FlagGroup.SetText(XorStr("Fake Lag"));
	FlagGroup.SetSize(293, 140);
	FlagGroup.SetPosition(333, 122);
	RegisterControl(&FlagGroup);

	FlagEnable.SetFileId(XorStr("m_flagenabled"));
	FlagGroup.PlaceLabeledControl(0, XorStr("Fake lag"), this, &FlagEnable);

	FlagActivationType.SetFileId(XorStr("m_flagactivationtype"));
	FlagActivationType.AddItem(XorStr("Always"));
	FlagActivationType.AddItem(XorStr("While moving"));
	FlagActivationType.AddItem(XorStr("In air"));
	FlagGroup.PlaceLabeledControl(0, XorStr(""), this, &FlagActivationType);

	FlagType.SetFileId(XorStr("m_flagtype"));
	FlagType.AddItem(XorStr("Off"));
	FlagType.AddItem(XorStr("Factor"));
	FlagType.AddItem(XorStr("Step up"));
	FlagType.AddItem(XorStr("Dynamic"));
	FlagGroup.PlaceLabeledControl(0, XorStr(""), this, &FlagType);

	FlagLimit.SetFileId(XorStr("m_flaglimit"));
	FlagLimit.SetBoundaries(0.f, 14.f);
	FlagLimit.SetValue(0.f);
	FlagGroup.PlaceLabeledControl(0, XorStr("Limit"), this, &FlagLimit);

	DisableFlagWhileShooting.SetFileId(XorStr("m_disableflagwhileshooting"));
	FlagGroup.PlaceLabeledControl(0, XorStr("Disable while shooting"), this, &DisableFlagWhileShooting);

	//Configs
	ConfigGroup.SetText(XorStr("Configs"));
	ConfigGroup.SetSize(293, 278);
	ConfigGroup.SetPosition(333, 278);
	RegisterControl(&ConfigGroup);

	ConfigListBox.SetHeightInItems(7);
	list_configs();
	ConfigGroup.PlaceLabeledControl(0, XorStr(""), this, &ConfigListBox);

	LoadConfig.SetText(XorStr("Load"));
	LoadConfig.SetCallback(&load_callback);
	ConfigGroup.PlaceLabeledControl(0, "", this, &LoadConfig);

	SaveConfig.SetText(XorStr("Save"));
	SaveConfig.SetCallback(&save_callback);
	ConfigGroup.PlaceLabeledControl(0, "", this, &SaveConfig);

	RemoveConfig.SetText(XorStr("Remove"));
	RemoveConfig.SetCallback(&remove_config);
	ConfigGroup.PlaceLabeledControl(0, "", this, &RemoveConfig);

	ConfigGroup.PlaceLabeledControl(0, "", this, &NewConfigName);

	AddConfig.SetText(XorStr("Add"));
	AddConfig.SetCallback(&add_config);
	ConfigGroup.PlaceLabeledControl(0, "", this, &AddConfig);
}

void CSkinsTab::Setup()
{
	SetTitle(XorStr("e"));

	OverrideModelGroup.SetText(XorStr("Knife Changer"));
	OverrideModelGroup.SetSize(293, 349);
	OverrideModelGroup.SetPosition(24, 16);
	RegisterControl(&OverrideModelGroup);

	SkinChangerEnabled.SetFileId(XorStr("s_enabled"));
	OverrideModelGroup.PlaceLabeledControl(0, XorStr("Enable"), this, &SkinChangerEnabled);

	OverrideKnife.SetFileId(XorStr("s_overrideknife"));
	OverrideModelGroup.PlaceLabeledControl(0, "Override Knife", this, &OverrideKnife);

	KnifeSelection.SetFileId(XorStr("s_knife"));
	KnifeSelection.AddItem(XorStr("Default"));
	KnifeSelection.AddItem(XorStr("Bayonet"));
	KnifeSelection.AddItem(XorStr("M9 Bayonet"));
	KnifeSelection.AddItem(XorStr("Butterfly"));
	KnifeSelection.AddItem(XorStr("Flip"));
	KnifeSelection.AddItem(XorStr("Gut"));
	KnifeSelection.AddItem(XorStr("Karambit"));
	KnifeSelection.AddItem(XorStr("Huntsman"));
	KnifeSelection.AddItem(XorStr("Falchion"));
	KnifeSelection.AddItem(XorStr("Shadow Daggers"));
	KnifeSelection.AddItem(XorStr("Bowie"));
	OverrideModelGroup.PlaceLabeledControl(0, XorStr(""), this, &KnifeSelection);

	KnifeSkin.SetFileId(XorStr("s_knifeskin"));
	KnifeSkin.AddItem(XorStr("Vanilla"), 0);
	KnifeSkin.AddItem(XorStr("Forest DDPAT"), 5);
	KnifeSkin.AddItem(XorStr("Doppler | Phase 1"), 418);
	KnifeSkin.AddItem(XorStr("Doppler | Phase 2"), 419);
	KnifeSkin.AddItem(XorStr("Doppler | Phase 3"), 420);
	KnifeSkin.AddItem(XorStr("Doppler | Phase 4"), 421);
	KnifeSkin.AddItem(XorStr("Doppler | Ruby"), 415);
	KnifeSkin.AddItem(XorStr("Doppler | Sapphire"), 416);
	KnifeSkin.AddItem(XorStr("Doppler | Blackpearl"), 417);
	KnifeSkin.AddItem(XorStr("Crimson Web"), 12);
	KnifeSkin.AddItem(XorStr("Slaughter"), 59);
	KnifeSkin.AddItem(XorStr("Fade"), 38);
	KnifeSkin.AddItem(XorStr("Night"), 40);
	KnifeSkin.AddItem(XorStr("Blue Steel"), 42);
	KnifeSkin.AddItem(XorStr("Stained"), 43);
	KnifeSkin.AddItem(XorStr("Case Hardened"), 44);
	KnifeSkin.AddItem(XorStr("Safari Mesh"), 72);
	KnifeSkin.AddItem(XorStr("Boreal Forest"), 77);
	KnifeSkin.AddItem(XorStr("Ultraviolet"), 98);
	KnifeSkin.AddItem(XorStr("Urban Masked"), 143);
	KnifeSkin.AddItem(XorStr("Damascus Steel"), 410);
	KnifeSkin.AddItem(XorStr("Scorched"), 175);
	KnifeSkin.AddItem(XorStr("Tiger Tooth"), 409);
	KnifeSkin.AddItem(XorStr("Marble Fade"), 413);
	KnifeSkin.AddItem(XorStr("Rust Coat"), 323);
	KnifeSkin.AddItem(XorStr("Bright Water"), 578);
	KnifeSkin.AddItem(XorStr("Lore | Bayonet"), 558);
	KnifeSkin.AddItem(XorStr("Lore | Flip"), 559);
	KnifeSkin.AddItem(XorStr("Lore | Gut"), 560);
	KnifeSkin.AddItem(XorStr("Lore | Karambit"), 561);
	KnifeSkin.AddItem(XorStr("Lore | M9"), 562);
	KnifeSkin.AddItem(XorStr("Autotronic | Bayonet"), 573);
	KnifeSkin.AddItem(XorStr("Autotronic | Flip"), 574);
	KnifeSkin.AddItem(XorStr("Autotronic | Gut"), 575);
	KnifeSkin.AddItem(XorStr("Autotronic | Karambit"), 576);
	KnifeSkin.AddItem(XorStr("Autotronic | M9"), 577);
	KnifeSkin.AddItem(XorStr("Gamma Doppler | Phase 1"), 569);
	KnifeSkin.AddItem(XorStr("Gamma Doppler | Phase 2"), 570);
	KnifeSkin.AddItem(XorStr("Gamma Doppler | Phase 3"), 571);
	KnifeSkin.AddItem(XorStr("Gamma Doppler | Phase 4"), 572);
	KnifeSkin.AddItem(XorStr("Gamma Doppler | Emerald"), 568);
	KnifeSkin.AddItem(XorStr("Black Laminate | Bayonet"), 563);
	KnifeSkin.AddItem(XorStr("Black Laminate | Flip"), 564);
	KnifeSkin.AddItem(XorStr("Black Laminate | Gut"), 565);
	KnifeSkin.AddItem(XorStr("Black Laminate | Karambit"), 566);
	KnifeSkin.AddItem(XorStr("Black Laminate | M9"), 567);
	KnifeSkin.AddItem(XorStr("Freehand | Phase 1"), 581);
	KnifeSkin.AddItem(XorStr("Freehand | Phase 2"), 582);
	KnifeSkin.SetHeightInItems(10);
	OverrideModelGroup.PlaceLabeledControl(0, XorStr(""), this, &KnifeSkin);

	KnifeWear.SetFileId(XorStr("s_knifewear"));
	KnifeWear.SetBoundaries(0, 1);
	KnifeWear.SetValue(0.01);
	KnifeWear.SetFormat(SliderFormat::FORMAT_DECDIG2);
	OverrideModelGroup.PlaceLabeledControl(0, XorStr("Fallback wear"), this, &KnifeWear);

	KnifeFullUpdate.SetText(XorStr("Update"));
	KnifeFullUpdate.SetCallback(&KnifeApplyCallbk);
	OverrideModelGroup.PlaceLabeledControl(0, XorStr(""), this, &KnifeFullUpdate);
}

void CPlayersTab::Setup()
{
	SetTitle(XorStr("f"));

	PlayerListControl.SetText(XorStr("Players"));
	PlayerListControl.SetSize(293, 540);
	PlayerListControl.SetPosition(24, 16);
	PlayerListControl.SetHeightPlayersTab(36);
	RegisterControl(&PlayerListControl);

	PlayerSettingsGroup.SetText(XorStr("Settings"));
	PlayerSettingsGroup.SetSize(293, 150);
	PlayerSettingsGroup.SetPosition(333, 16);
	RegisterControl(&PlayerSettingsGroup);

	PlayerForcePitch_Pitch.AddItem(XorStr("Eye angles"));
	PlayerForcePitch_Pitch.AddItem(XorStr("Up"));
	PlayerForcePitch_Pitch.AddItem(XorStr("Emotion"));

	PlayerForceYaw_Yaw.AddItem(XorStr("Default"));
	PlayerForceYaw_Yaw.AddItem(XorStr("Correction"));
	PlayerForceYaw_Yaw.AddItem(XorStr("Eye angles"));
	PlayerForceYaw_Yaw.AddItem(XorStr("Sideways left"));
	PlayerForceYaw_Yaw.AddItem(XorStr("Sideways right"));
	PlayerForceYaw_Yaw.AddItem(XorStr("180°"));

	PlayerSettingsGroup.PlaceLabeledControl(0, XorStr("Priority"), this, &PlayerPriority);
	PlayerSettingsGroup.PlaceLabeledControl(0, XorStr("Whitelist"), this, &PlayerFriendly);
	PlayerSettingsGroup.PlaceLabeledControl(0, XorStr("Prioritize body"), this, &PlayerPreferBody);
	PlayerSettingsGroup.PlaceLabeledControl(0, XorStr("Force pitch"), this, &PlayerForcePitch);
	PlayerSettingsGroup.PlaceLabeledControl(0, XorStr(""), this, &PlayerForcePitch_Pitch);
	PlayerSettingsGroup.PlaceLabeledControl(0, XorStr("Force yaw"), this, &PlayerForceYaw);
	PlayerSettingsGroup.PlaceLabeledControl(0, XorStr(""), this, &PlayerForceYaw_Yaw);
}

void Menu::SetupMenu()
{
	Window.Setup();

	GUI.RegisterWindow(&Window);
	GUI.BindWindow(VK_INSERT, &Window);
}

void Menu::DoFrame()
{
	GUI.Update();
	GUI.Draw();

	plist.paint();
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class gymwwlf
{
public:
	bool cwpfatrgrl;
	string rvnwkeobmzfykth;
	string vudxbrqgohshu;
	bool ukuakzvrygyvt;
	string rvttbyux;
	gymwwlf();
	double ecxrgahcyhcwnzsx(double npzfpupdypllp, string teprwyxmo, int dhpxjl, double ieffwploikmttm, string mnwsagxapatfe, bool oifwgdak, string kbmcioqhmfapf, double nkhepchpt, string aumryotix, int iarou);
	int yfpdigiour(int jzktssheel);
	int pygeoxabsfyi(string lrobrrliyddkn, double vqlayi);
	void nqanrfwudm(int vtrzrhc, bool rveiuv, bool fijtkpvrvopy);
	void eezysliqqcyrzqsgenr(double woezbcbbiwr, double auvaekzspinowr, bool iztvzsgktpsr, bool daifgmwhba, string sqijtqelib, int vfrkxjuaff, int djllakvmls, string yjnxxseykma, bool prrzrhnncrwu);
	int wtdbezweyogmwahcewcvy(int pbzvqfbavtkmll, double iexycizmpdjzag, int qsyacowwywemv, int gxpvfa, bool rivarslygbdght, string xnmhqudwhxhi, bool bmuuiksvskhlktv, int bsyayxpa, double moxml);
	double tfztvlsepdmcbgqnfnbz(bool ixzgoucehrtup, bool jwmsdieo, double hiwkwmsujol, int nagmqrrn, string xndlaskdep, string fnetbntdc, int mbsurhiokbup, int slnvwnazlqvwa, string tzqteapginlx);
	bool pknfpdvspqof(double lyduhmopcj, double ubimsbzfqat, int lyujjdospve, bool usaqfc, int oxuatoo);
	bool gxdppeelkmofstxpzwuohf(int crpjekpepi, double flvfiikkoiwbgi, string vdpuad, string gqfyu, int pdxwkpd, int ykdksiwajwulhgw);

protected:
	bool njtnre;
	double xehsrohwiy;

	int eqpcnvyucettpautcdhry(string ivzuzp, int kayqkclmmyhzlp, double lsdnhjkqz, string glnlnixna, int jsherartmjregt, bool ujwgsppxfotex, bool zyabgazodyrthxq, double pdqwtgdo, int omcqupklajruu, string mctqn);
	void znaammjibtczckuucjwlnen(bool mkrjgrvvlwl, bool guvspdwhlqvl, double yhorrosdyb, string wrwsrf);
	bool kfuphswgkhmtcir(int qkpkdccidjrp);

private:
	int agktmwo;
	int elexnsm;

	double chnzrjpoamkg(int nttggkaa, string ttbipveinlxzn, int rgyqbkspildqrxs, bool pyweezmzwik, bool tdojcpxq, bool mthzzdor, string rfukqecfax, string fneddlrxgc, double secxkc, double tulhpworxysfsi);
	int fiwblaqfpypdhpfxdbxxiijr(bool pxwirbqpmv);
	string fvkzcypwtuenrwaf(double skxiatgurlfffdq, int iiypnnnygpf, int buamgqslfpulik, int cykgmhqec, bool numyrkgagxcfn, int zbvoyhqovuejasn, bool crpndppx);
	string mnatkrlrrulli(double tgmogjaclbkwfxj, bool rfbmxtzunidev, int hjjnmcaonvuzfj, bool ivmetsdj, int tbdgsghnvg, string fwazddm, bool mwoodmulaai, int tkihfoluqj, double qjpdejmlvbqr);
	bool vkdsonbrwgspjguixupbawdl(string ywugmd, string cjxhhvxlpmmq, string qjbjvjs, bool jjsqc);
	string qjgtfnlksattjydembdxdn(double esjuqvvjrvohyx, int bcoyfjwon, bool oclfna, string apmuuqwydoouf, int ptyqqc);
	void mknngpicjnwhygmzwkjlgz(double ammrmqtwfgbud);
	double mwvqkacjiskgtfkzwytq(double vbtjbqzudv, int gsbyetgfxx, int mxvwjszhbm, string jvmkscl, int lihuhroninobzc, int rtkos);
	bool yadmrpqmovmaeododqss(double lvwpvrxxvw, string ptxaxwm, double hfpzivl, string wuhwbpult, bool tulyqrfxseeh, bool nlgmnkvv, bool oomoavejii, double aigvvqdwaiq, bool qbbtqpdyp);

};

double gymwwlf::chnzrjpoamkg(int nttggkaa, string ttbipveinlxzn, int rgyqbkspildqrxs, bool pyweezmzwik, bool tdojcpxq, bool mthzzdor, string rfukqecfax, string fneddlrxgc, double secxkc, double tulhpworxysfsi)
{
	double favoo = 53916;
	int bftuykkoo = 1600;
	if(53916 != 53916)
	{
		int nfazus;
		for(nfazus = 96; nfazus > 0; nfazus--)
		{
			continue;
		}
	}
	if(53916 != 53916)
	{
		int ogihvwjuk;
		for(ogihvwjuk = 11; ogihvwjuk > 0; ogihvwjuk--)
		{
			continue;
		}
	}
	if(53916 != 53916)
	{
		int vfuqxh;
		for(vfuqxh = 24; vfuqxh > 0; vfuqxh--)
		{
			continue;
		}
	}
	if(53916 != 53916)
	{
		int ldkc;
		for(ldkc = 35; ldkc > 0; ldkc--)
		{
			continue;
		}
	}
	return 18380;
}

int gymwwlf::fiwblaqfpypdhpfxdbxxiijr(bool pxwirbqpmv)
{
	int vyzkksumsizjc = 4963;
	bool zfxivoorn = true;
	string iertt = "n";
	bool wmduvmxacval = true;
	bool znhgogsd = true;
	bool cbjylpzfakn = true;
	if(true != true)
	{
		int jzvqsb;
		for(jzvqsb = 85; jzvqsb > 0; jzvqsb--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int nxjfkjzxb;
		for(nxjfkjzxb = 60; nxjfkjzxb > 0; nxjfkjzxb--)
		{
			continue;
		}
	}
	if(string("n") == string("n"))
	{
		int ygggnwbndu;
		for(ygggnwbndu = 19; ygggnwbndu > 0; ygggnwbndu--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int hwjobyfs;
		for(hwjobyfs = 88; hwjobyfs > 0; hwjobyfs--)
		{
			continue;
		}
	}
	return 34649;
}

string gymwwlf::fvkzcypwtuenrwaf(double skxiatgurlfffdq, int iiypnnnygpf, int buamgqslfpulik, int cykgmhqec, bool numyrkgagxcfn, int zbvoyhqovuejasn, bool crpndppx)
{
	double vtoituc = 10730;
	string mebuywnglxdk = "mxlqdvncdfjwqoigirpfhueahgsqczssqugdon";
	bool qdpqlsa = false;
	string lshsxanfhhauuxs = "wyuqxjxyhoslhzpskoizlpyoranapaneaqbaysejsloeqdxhxtkyrgnziwbpyfthesvyevpbwovthsxjsgrbnrpanwyvrusi";
	double wvdgigcoutb = 10372;
	string jrodamx = "nhj";
	if(string("wyuqxjxyhoslhzpskoizlpyoranapaneaqbaysejsloeqdxhxtkyrgnziwbpyfthesvyevpbwovthsxjsgrbnrpanwyvrusi") != string("wyuqxjxyhoslhzpskoizlpyoranapaneaqbaysejsloeqdxhxtkyrgnziwbpyfthesvyevpbwovthsxjsgrbnrpanwyvrusi"))
	{
		int ejfbotwalo;
		for(ejfbotwalo = 46; ejfbotwalo > 0; ejfbotwalo--)
		{
			continue;
		}
	}
	if(10372 != 10372)
	{
		int xmkpe;
		for(xmkpe = 52; xmkpe > 0; xmkpe--)
		{
			continue;
		}
	}
	if(10730 == 10730)
	{
		int zlxphllb;
		for(zlxphllb = 14; zlxphllb > 0; zlxphllb--)
		{
			continue;
		}
	}
	return string("");
}

string gymwwlf::mnatkrlrrulli(double tgmogjaclbkwfxj, bool rfbmxtzunidev, int hjjnmcaonvuzfj, bool ivmetsdj, int tbdgsghnvg, string fwazddm, bool mwoodmulaai, int tkihfoluqj, double qjpdejmlvbqr)
{
	int oqusdoq = 8726;
	bool tqodftqksmzlhnq = false;
	string ftzpqwolawf = "fucyhjsnswcnvtracjxvrnrxw";
	string msgqxqecbrye = "fzheiwybvhatnsgihwfbhihylprmweqnmvsccvpifxuuhbqebcdlojeyeqsmlrqnnycwglfcmufrodepex";
	double ezmirlnmhsll = 61840;
	int qohiawikeak = 6437;
	string xmfri = "objxavmemxlgaldgyjfjbh";
	return string("ibzmslhdikttugeox");
}

bool gymwwlf::vkdsonbrwgspjguixupbawdl(string ywugmd, string cjxhhvxlpmmq, string qjbjvjs, bool jjsqc)
{
	int vhgjkjyr = 1244;
	int jsygcmqqmtqpbcn = 5725;
	bool qgplisgk = false;
	int zopnjwbxkqychh = 9266;
	int bpopftvpxusemfp = 5036;
	bool zgofwmelumgez = false;
	string eptbjkvpsi = "bgeifhwwkkougedclbgbxmkstyklisfcwhibuxpkqcnwxnfvhigslgbyeuiendwhkwwxpzjwsbapyfvh";
	if(9266 == 9266)
	{
		int swma;
		for(swma = 66; swma > 0; swma--)
		{
			continue;
		}
	}
	if(5725 != 5725)
	{
		int qqxjuqqfvo;
		for(qqxjuqqfvo = 68; qqxjuqqfvo > 0; qqxjuqqfvo--)
		{
			continue;
		}
	}
	return true;
}

string gymwwlf::qjgtfnlksattjydembdxdn(double esjuqvvjrvohyx, int bcoyfjwon, bool oclfna, string apmuuqwydoouf, int ptyqqc)
{
	int xhdyykth = 3279;
	string fprdrfymiwjrzq = "leumlzdrekfffjliudoithpqbevlxngokocivyncexcccwzvoltyesbuqglqfaljoszoegbfynkickaxxpjzl";
	if(string("leumlzdrekfffjliudoithpqbevlxngokocivyncexcccwzvoltyesbuqglqfaljoszoegbfynkickaxxpjzl") == string("leumlzdrekfffjliudoithpqbevlxngokocivyncexcccwzvoltyesbuqglqfaljoszoegbfynkickaxxpjzl"))
	{
		int qqwwvpd;
		for(qqwwvpd = 37; qqwwvpd > 0; qqwwvpd--)
		{
			continue;
		}
	}
	if(3279 != 3279)
	{
		int qranx;
		for(qranx = 52; qranx > 0; qranx--)
		{
			continue;
		}
	}
	if(string("leumlzdrekfffjliudoithpqbevlxngokocivyncexcccwzvoltyesbuqglqfaljoszoegbfynkickaxxpjzl") != string("leumlzdrekfffjliudoithpqbevlxngokocivyncexcccwzvoltyesbuqglqfaljoszoegbfynkickaxxpjzl"))
	{
		int wfbruvy;
		for(wfbruvy = 2; wfbruvy > 0; wfbruvy--)
		{
			continue;
		}
	}
	return string("vnnjkq");
}

void gymwwlf::mknngpicjnwhygmzwkjlgz(double ammrmqtwfgbud)
{
	bool ifuukxawzz = false;
	double hyoiztcqklr = 14025;
	int ernbnpntoryy = 3132;
	string qxjfwkvwwvfkrlb = "iswbvytobvrrifrybnquqmfllsypmhdfjjhqfknqokgncfv";
	if(3132 != 3132)
	{
		int eldam;
		for(eldam = 10; eldam > 0; eldam--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int zykgbf;
		for(zykgbf = 11; zykgbf > 0; zykgbf--)
		{
			continue;
		}
	}

}

double gymwwlf::mwvqkacjiskgtfkzwytq(double vbtjbqzudv, int gsbyetgfxx, int mxvwjszhbm, string jvmkscl, int lihuhroninobzc, int rtkos)
{
	int uqwpxn = 1869;
	double gnvcimvfvky = 4443;
	string rdtdt = "ihjjtmnznxqudughlhatvdaoztkoudcxdbuxqbcciyaf";
	int alwvepxw = 911;
	bool obhbijcmqvoe = true;
	double txrsgudktixnm = 40648;
	bool kcnwojbfqo = true;
	double oizowacpbchouo = 36619;
	if(40648 != 40648)
	{
		int gnodmuibfc;
		for(gnodmuibfc = 86; gnodmuibfc > 0; gnodmuibfc--)
		{
			continue;
		}
	}
	return 49400;
}

bool gymwwlf::yadmrpqmovmaeododqss(double lvwpvrxxvw, string ptxaxwm, double hfpzivl, string wuhwbpult, bool tulyqrfxseeh, bool nlgmnkvv, bool oomoavejii, double aigvvqdwaiq, bool qbbtqpdyp)
{
	double hcxvfv = 7641;
	int xfaynepdstmwsy = 297;
	double ptwndohzpyc = 21286;
	string cnldskxgjo = "bfifhszlcfltheogpozpqxarwekprqtbjhmnjhtusyrgmqxckvfpsbttrsolehifibsbdognqctmicbfadbg";
	string ztjnqvktxqqt = "zhzvpgqrsiaasuxxfohbbmjabdkcobfxtnazwattwgtyrksdkcwsechzuloo";
	string bendjto = "ssyhxdlibqqyrhyxamiomihkoejtuclmnyxaxsngeqchvahheeqzjkdonlegusoyftinqsirwtimtthlgjplcwytsjljyn";
	string cmypllvnzhssbk = "retvbtzezpnzcohgtxqyvnhokahjofhiprfukigwxbacbdexbsjqzprxwjwbqclqztpddgdwjwcm";
	string gypbwhidjutfze = "ofrtilkzoudpqpyfwsljiyxrpgaogqsyqicylbothbqwqymkcsglzvwvijpckmdiadljqzijkmrbxratalmdicubov";
	bool eeywteals = false;
	int jkndfenalz = 926;
	return false;
}

int gymwwlf::eqpcnvyucettpautcdhry(string ivzuzp, int kayqkclmmyhzlp, double lsdnhjkqz, string glnlnixna, int jsherartmjregt, bool ujwgsppxfotex, bool zyabgazodyrthxq, double pdqwtgdo, int omcqupklajruu, string mctqn)
{
	bool odulcsvuddtu = true;
	int ahnudgjfgakvcjp = 4817;
	int stfyozyi = 2100;
	int talqwc = 962;
	bool jpqyiei = true;
	string huoehtwvfa = "bguasdhlnkmwtyojkeppjemwtuswcgvoyiqieqfxfaoaibgwbmyuuwtomdwfierzarslsocunnadjpubtbismxvvhalc";
	int ysffzorfbymkqav = 2253;
	int qymey = 1876;
	double htnysrrg = 33515;
	bool azcdhvrtlozrxmo = false;
	if(true != true)
	{
		int qwzuiypx;
		for(qwzuiypx = 83; qwzuiypx > 0; qwzuiypx--)
		{
			continue;
		}
	}
	if(string("bguasdhlnkmwtyojkeppjemwtuswcgvoyiqieqfxfaoaibgwbmyuuwtomdwfierzarslsocunnadjpubtbismxvvhalc") == string("bguasdhlnkmwtyojkeppjemwtuswcgvoyiqieqfxfaoaibgwbmyuuwtomdwfierzarslsocunnadjpubtbismxvvhalc"))
	{
		int tmsuznj;
		for(tmsuznj = 62; tmsuznj > 0; tmsuznj--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int vntoruwx;
		for(vntoruwx = 35; vntoruwx > 0; vntoruwx--)
		{
			continue;
		}
	}
	if(2100 != 2100)
	{
		int zghijked;
		for(zghijked = 96; zghijked > 0; zghijked--)
		{
			continue;
		}
	}
	return 36298;
}

void gymwwlf::znaammjibtczckuucjwlnen(bool mkrjgrvvlwl, bool guvspdwhlqvl, double yhorrosdyb, string wrwsrf)
{
	double ahehcqqqtpbgp = 4393;
	bool azcsnyxqgzem = true;
	bool nzncbg = false;
	string ngjtwfhywas = "mcjffgolwaivsxxuyqhyhcmebppnyqvh";
	if(string("mcjffgolwaivsxxuyqhyhcmebppnyqvh") != string("mcjffgolwaivsxxuyqhyhcmebppnyqvh"))
	{
		int uqi;
		for(uqi = 64; uqi > 0; uqi--)
		{
			continue;
		}
	}
	if(string("mcjffgolwaivsxxuyqhyhcmebppnyqvh") != string("mcjffgolwaivsxxuyqhyhcmebppnyqvh"))
	{
		int evvthpifsk;
		for(evvthpifsk = 37; evvthpifsk > 0; evvthpifsk--)
		{
			continue;
		}
	}
	if(string("mcjffgolwaivsxxuyqhyhcmebppnyqvh") == string("mcjffgolwaivsxxuyqhyhcmebppnyqvh"))
	{
		int xtfmc;
		for(xtfmc = 84; xtfmc > 0; xtfmc--)
		{
			continue;
		}
	}
	if(true == true)
	{
		int jyhfyu;
		for(jyhfyu = 10; jyhfyu > 0; jyhfyu--)
		{
			continue;
		}
	}

}

bool gymwwlf::kfuphswgkhmtcir(int qkpkdccidjrp)
{
	int xjdispmlzhd = 2909;
	string mptflviaillyfxi = "ybpneoiidrbcvlsklzbysqjihfeqoszymhdxgqhbklgvxnswaoscov";
	string jyckjwzflumj = "or";
	string cxibvxh = "xqyunghfbqoylzxajxrqlbtxnwigkpgshbtadcwjnqtezafcik";
	if(string("xqyunghfbqoylzxajxrqlbtxnwigkpgshbtadcwjnqtezafcik") != string("xqyunghfbqoylzxajxrqlbtxnwigkpgshbtadcwjnqtezafcik"))
	{
		int jho;
		for(jho = 12; jho > 0; jho--)
		{
			continue;
		}
	}
	if(2909 != 2909)
	{
		int qi;
		for(qi = 85; qi > 0; qi--)
		{
			continue;
		}
	}
	return true;
}

double gymwwlf::ecxrgahcyhcwnzsx(double npzfpupdypllp, string teprwyxmo, int dhpxjl, double ieffwploikmttm, string mnwsagxapatfe, bool oifwgdak, string kbmcioqhmfapf, double nkhepchpt, string aumryotix, int iarou)
{
	return 21127;
}

int gymwwlf::yfpdigiour(int jzktssheel)
{
	string ihveaepfggecga = "htfwkrbvplclwlmhwvtxgaudc";
	double vzvpbc = 64499;
	double nwwwmeczivqmhfv = 51441;
	double opzldow = 26846;
	string hzgkfm = "nnxxsrbziecuqztbvxapqqusfdmsmqwgwhrybwbfjnfoebkscwizmnoaezspnrrebgvtkhwlhygbgonqyawkrwjhsav";
	if(26846 == 26846)
	{
		int evpsflgcs;
		for(evpsflgcs = 35; evpsflgcs > 0; evpsflgcs--)
		{
			continue;
		}
	}
	if(26846 != 26846)
	{
		int fef;
		for(fef = 51; fef > 0; fef--)
		{
			continue;
		}
	}
	if(string("htfwkrbvplclwlmhwvtxgaudc") != string("htfwkrbvplclwlmhwvtxgaudc"))
	{
		int tvjttr;
		for(tvjttr = 6; tvjttr > 0; tvjttr--)
		{
			continue;
		}
	}
	return 77546;
}

int gymwwlf::pygeoxabsfyi(string lrobrrliyddkn, double vqlayi)
{
	double orpjn = 8811;
	int lnhabn = 7016;
	if(7016 != 7016)
	{
		int gzzajwe;
		for(gzzajwe = 70; gzzajwe > 0; gzzajwe--)
		{
			continue;
		}
	}
	if(7016 == 7016)
	{
		int rexbmldzb;
		for(rexbmldzb = 14; rexbmldzb > 0; rexbmldzb--)
		{
			continue;
		}
	}
	if(8811 == 8811)
	{
		int yxdahbrrws;
		for(yxdahbrrws = 39; yxdahbrrws > 0; yxdahbrrws--)
		{
			continue;
		}
	}
	return 15511;
}

void gymwwlf::nqanrfwudm(int vtrzrhc, bool rveiuv, bool fijtkpvrvopy)
{
	int hwhwypwe = 1564;
	string qlztmaurtfef = "souypcqlamhbvbylhgungukkljb";
	int jqsaspmydn = 574;
	bool xqkejwgftasrn = true;
	bool wjlgbglfi = false;
	int fiityeom = 1050;
	string zxbzyawexqj = "soncvwgncdtqhrntlgobvilaxhlsopodbtzzdzwmyxgiuinoccrddadoclucvfoghatqpoxrifsdgqfjqggrtkmhjstsgqmbn";
	string wqtufj = "ikottanfddauhkhaflvrozpkrwedfubgbnfzayzkdidvnkwugsrjwzmkaofypjyymvzmxmhellcaysbyglhhg";
	double gxcosk = 44915;
	string vwtwqobael = "pddomsbhddmz";
	if(string("souypcqlamhbvbylhgungukkljb") == string("souypcqlamhbvbylhgungukkljb"))
	{
		int oo;
		for(oo = 75; oo > 0; oo--)
		{
			continue;
		}
	}
	if(string("pddomsbhddmz") == string("pddomsbhddmz"))
	{
		int kfmvmaq;
		for(kfmvmaq = 83; kfmvmaq > 0; kfmvmaq--)
		{
			continue;
		}
	}

}

void gymwwlf::eezysliqqcyrzqsgenr(double woezbcbbiwr, double auvaekzspinowr, bool iztvzsgktpsr, bool daifgmwhba, string sqijtqelib, int vfrkxjuaff, int djllakvmls, string yjnxxseykma, bool prrzrhnncrwu)
{
	bool gufwkak = true;
	bool cnzeeewjuxydan = true;
	int atjyhjiuxmmgfvl = 5199;
	bool hpxgcb = true;
	double ewagyqxmqbg = 25844;
	int apnelk = 2504;
	string vwtco = "mnjdnxhudmkzmqizdphnikovumygyiercuvzjavsyqduvpvjrujzmoshfvlgitjchdszdueixyqaohzdkfrnfwa";
	int ogrgvrhibhnkyxp = 1452;
	if(5199 != 5199)
	{
		int vpanzfs;
		for(vpanzfs = 68; vpanzfs > 0; vpanzfs--)
		{
			continue;
		}
	}
	if(1452 != 1452)
	{
		int ruokfx;
		for(ruokfx = 68; ruokfx > 0; ruokfx--)
		{
			continue;
		}
	}

}

int gymwwlf::wtdbezweyogmwahcewcvy(int pbzvqfbavtkmll, double iexycizmpdjzag, int qsyacowwywemv, int gxpvfa, bool rivarslygbdght, string xnmhqudwhxhi, bool bmuuiksvskhlktv, int bsyayxpa, double moxml)
{
	double cqjegdn = 2940;
	string lgwyhjvvonb = "rgozxatqhybgugodlvrgowueaeiakvbepcoxplqliphhtivsstciytsvtdjmjdbcsmmpekvrivf";
	if(2940 != 2940)
	{
		int af;
		for(af = 19; af > 0; af--)
		{
			continue;
		}
	}
	if(string("rgozxatqhybgugodlvrgowueaeiakvbepcoxplqliphhtivsstciytsvtdjmjdbcsmmpekvrivf") == string("rgozxatqhybgugodlvrgowueaeiakvbepcoxplqliphhtivsstciytsvtdjmjdbcsmmpekvrivf"))
	{
		int apon;
		for(apon = 9; apon > 0; apon--)
		{
			continue;
		}
	}
	return 17451;
}

double gymwwlf::tfztvlsepdmcbgqnfnbz(bool ixzgoucehrtup, bool jwmsdieo, double hiwkwmsujol, int nagmqrrn, string xndlaskdep, string fnetbntdc, int mbsurhiokbup, int slnvwnazlqvwa, string tzqteapginlx)
{
	string srppzrvzexvo = "hmijjqbateu";
	bool qkkkh = false;
	string pmmjzhcixkcm = "chdjygmpqoupmrinzdoxrkqkpchjnnrqoutajassmcbaotudjcqyaxvhsrcphwecstwkspezklzgviktrhuwondmekkydsa";
	bool znhah = true;
	int puvwgaxswg = 1324;
	string tpxvaaeykh = "ngtgdafbtilfcwrjutqxbfxefvfxmzwealrjpndmwcdzvrvghsabcmdah";
	bool uauxabjehwyhii = false;
	double gexiczcyqzrq = 73445;
	double ovpamxulx = 7986;
	double wjpsazsh = 32182;
	if(73445 != 73445)
	{
		int mqj;
		for(mqj = 88; mqj > 0; mqj--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int vxjnilw;
		for(vxjnilw = 100; vxjnilw > 0; vxjnilw--)
		{
			continue;
		}
	}
	if(string("chdjygmpqoupmrinzdoxrkqkpchjnnrqoutajassmcbaotudjcqyaxvhsrcphwecstwkspezklzgviktrhuwondmekkydsa") == string("chdjygmpqoupmrinzdoxrkqkpchjnnrqoutajassmcbaotudjcqyaxvhsrcphwecstwkspezklzgviktrhuwondmekkydsa"))
	{
		int dt;
		for(dt = 21; dt > 0; dt--)
		{
			continue;
		}
	}
	return 93181;
}

bool gymwwlf::pknfpdvspqof(double lyduhmopcj, double ubimsbzfqat, int lyujjdospve, bool usaqfc, int oxuatoo)
{
	return false;
}

bool gymwwlf::gxdppeelkmofstxpzwuohf(int crpjekpepi, double flvfiikkoiwbgi, string vdpuad, string gqfyu, int pdxwkpd, int ykdksiwajwulhgw)
{
	double exfogfuzjvoj = 43913;
	int oltioxguzejqi = 2047;
	double pwveeknadtp = 13363;
	bool qhpjn = false;
	int tvvomdcuv = 3522;
	bool dnfst = false;
	double ndlqviyrbnbgoe = 44753;
	if(false != false)
	{
		int booai;
		for(booai = 52; booai > 0; booai--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int fugk;
		for(fugk = 2; fugk > 0; fugk--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int peffvu;
		for(peffvu = 43; peffvu > 0; peffvu--)
		{
			continue;
		}
	}
	return true;
}

gymwwlf::gymwwlf()
{
	this->ecxrgahcyhcwnzsx(6039, string("hayiwknutjcopjwwnzlosoflhegxgerwuaivbkakbvfelbkidnjlqkihfqlzhhfnzsqfqe"), 952, 31316, string("innkxvdpqmpctzvdxjlxjgbxatefugfikiul"), true, string("wvealihyaqsocdydnfopfdzvshtgsjkklnutdurrwzopvkynzjtdoawjjmjnulkccriofuqpylemmlzdqmdonjtagdxarpxlta"), 28289, string("ltwvcajablqrcbsvcozxfrlmbmgkbtegyeslxmwyygislvlubpagfz"), 2648);
	this->yfpdigiour(275);
	this->pygeoxabsfyi(string("gfkzwcbwvzwkgbuykgeaoqxusnqpeyrhzepoatfqgycmkhmpsobhznsacs"), 42274);
	this->nqanrfwudm(663, false, false);
	this->eezysliqqcyrzqsgenr(9551, 22475, false, true, string("kqdwlgcrrferpaiunzuwvuallaqtllbtdlsahxtkubynkpcpzhn"), 1839, 473, string("ipq"), false);
	this->wtdbezweyogmwahcewcvy(5116, 12549, 533, 5039, false, string("pyussezdsypcoxbtsnwblsoymba"), true, 205, 21591);
	this->tfztvlsepdmcbgqnfnbz(true, false, 35893, 1906, string("nwaihwsesrapxmmqsriorbyxasnhzgqqphlfbnii"), string("yvltyvqahxxcfoyshyglvdhhfnodadnwcfqrbombyfnrfexclctgfcvefokruux"), 436, 980, string("ytjuxmjdwdfhxoejewdduny"));
	this->pknfpdvspqof(23252, 88005, 2940, true, 5069);
	this->gxdppeelkmofstxpzwuohf(4761, 11958, string("qwaxc"), string("mcddlpkzebkwmbysnziedykkqffdf"), 1800, 654);
	this->eqpcnvyucettpautcdhry(string("mwuxnqlqrbwwyukb"), 5530, 43558, string("rgyajvvzhuacyfjudyqekotdzqkhudsbmtam"), 1504, true, false, 34595, 2932, string("ldkjtiwtpphiesgnhc"));
	this->znaammjibtczckuucjwlnen(true, true, 13129, string("tfk"));
	this->kfuphswgkhmtcir(891);
	this->chnzrjpoamkg(9828, string("nioblngxvnejolottdvejhuxwoehhalcwxizwyzvskjvqefofgfnchphnrzrntejwevydteejiphxorjlekfxegc"), 1475, true, false, false, string("fyvggzotgdbzeouiqzarueufmkiwejbqmlclqdnswpjnvgohvkcijudlteeeenvrjyugfshpnkct"), string("oyo"), 49977, 32809);
	this->fiwblaqfpypdhpfxdbxxiijr(true);
	this->fvkzcypwtuenrwaf(5753, 1333, 2620, 7908, false, 2748, true);
	this->mnatkrlrrulli(3616, false, 1527, false, 520, string("kfmzwvxlpvuqcmeioftlqqckziiesyrz"), false, 4035, 23112);
	this->vkdsonbrwgspjguixupbawdl(string("aliumwhi"), string("vvwirtansgomsyuvwrykulzsbyzxbtp"), string("vxenfivhqrdbbdzkwdnjtocndnfbgmbwfuibtypdildwfvwldqgsuyixgzhsrflrnhehw"), true);
	this->qjgtfnlksattjydembdxdn(41458, 169, true, string("mjljqdtqvukxrimxwnphnydhbqkzbxtrewzyf"), 4927);
	this->mknngpicjnwhygmzwkjlgz(4310);
	this->mwvqkacjiskgtfkzwytq(58486, 2081, 1252, string(""), 3985, 1676);
	this->yadmrpqmovmaeododqss(70860, string("abvvetestqnuqjturvnemxqphudrctkmznyodbhuatduimytwrqrvmwlwpsrddkszkpsopcgdupr"), 57361, string("tzlfeiohlhkkysakwqbofsxdlmldj"), true, true, false, 54912, true);
}

#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

class zkbeitc
{
public:
	double fzznnlm;
	int dggnfhpgddqxw;
	int dyceebg;
	zkbeitc();
	void pmocxzawobikkct(bool gwndyccgvwi, double xwbqe, string txxvpniqtkv, int qahqrwf, double ypjkzrnjybuowo);
	int lrmaaaobyppk(int lswiawminvzmpi, int fveyegpa, double uablffpofgglppn, int mbhdqgcyzn, double dtbbljypqnupbh);
	string fnunssdurgryxoqafveq(int odopjke, int wsbwurpasczeu, string ozjozj, int frnvbrdhdsx, int ybzlhokdjnlh, int mkzzfsrfulotkx, string enjiggvv, bool bkdpqeopqibkezl, int qasdbkiyv, string zmtms);
	void uhdksemrttpqbs(bool ixmoijxvxg, string jkprjp, double fgdfefazu, int ukswhriqbkukkao);
	int fjjraglagnhmmze(bool wwgyj, string loaybta, string kqheopidnrz, double ukouufar, string optjtntusjtjyn, string ikzpepqwc, double frsexkzykfxca, string dldqzvqendyr, bool ayobdmlhlxuqw);
	double qkwdxbqrecgcvmwzm(bool fzjavsmsqgv, double hxqpujnrimak);

protected:
	bool skyaozw;
	string xzgiaptfygrtk;
	bool vbejq;
	double wjdkuqfgpldvklb;
	int vyobpj;

	double bvszvskaofxe(int ghqmvgji, string avgigaij, string itmwdtkhrvajhne, string mjoihbf);
	bool wdwvzuzijhcrygppaol();
	bool ptaqmarnhtwvotphiobhyojh(double lrlflulqei, int ebfihwzjdjoinm, double oohqvqzlgvwgckv, string tltpb, bool eqwyuzsmopolln, int aspmmjh);

private:
	int mavqqhztuwojqt;
	bool htcoawzihaasbp;
	string jeirkhwwuiquysl;
	double cknwhlfd;
	bool usshrfrmew;

	bool hqohdtwlijatlhizw(bool mhjsakrvajy, bool txozqcishqaajmm, double zaqtritiga, bool mekedxbomvhla, double dyrgavxye, double gqerups, bool lslgnvlg, bool gbhrzbyw);
	bool relnzcmljicufixmhiwly(double hawaeb, double nxrtptznwly, string eshovinaytnyw, double oxbfdp, string cfpgccgfxhxe, bool wpcqupyyefnkbo, int wihxqbyupmyklk, bool kjmyeyrsvezqmb, double ynvssffshugnhqt);
	bool ufphfchtgrkaxkaxqumak(bool qiixfklmonlyzo, bool dxnryhxqbuy, int tmnjrv, bool ecpyf, string gzndilq);
	string pcvfyhgbsagjah();

};



bool zkbeitc::hqohdtwlijatlhizw(bool mhjsakrvajy, bool txozqcishqaajmm, double zaqtritiga, bool mekedxbomvhla, double dyrgavxye, double gqerups, bool lslgnvlg, bool gbhrzbyw)
{
	int yzvydfson = 561;
	double gklqmdk = 33753;
	int vqdznacvwaegh = 1391;
	string htmgtbdrjncc = "cyzvuhphabwjiesflms";
	bool wgyjvarudb = false;
	if(false == false)
	{
		int nv;
		for(nv = 8; nv > 0; nv--)
		{
			continue;
		}
	}
	if(561 != 561)
	{
		int lmbkxwlgoy;
		for(lmbkxwlgoy = 4; lmbkxwlgoy > 0; lmbkxwlgoy--)
		{
			continue;
		}
	}
	if(33753 != 33753)
	{
		int ubsa;
		for(ubsa = 73; ubsa > 0; ubsa--)
		{
			continue;
		}
	}
	if(561 != 561)
	{
		int ywx;
		for(ywx = 59; ywx > 0; ywx--)
		{
			continue;
		}
	}
	if(33753 == 33753)
	{
		int gypod;
		for(gypod = 4; gypod > 0; gypod--)
		{
			continue;
		}
	}
	return false;
}

bool zkbeitc::relnzcmljicufixmhiwly(double hawaeb, double nxrtptznwly, string eshovinaytnyw, double oxbfdp, string cfpgccgfxhxe, bool wpcqupyyefnkbo, int wihxqbyupmyklk, bool kjmyeyrsvezqmb, double ynvssffshugnhqt)
{
	int omzopx = 514;
	string rcsrwyqprsyhx = "lixxiosmaavze";
	double wlurkepd = 49641;
	double hgzhjhajkpd = 4244;
	string ofkqnkjigngxeyg = "";
	bool fmmpcxahf = false;
	double zghpxnpy = 54967;
	string ednewggidr = "djjlxjrjrkqohsyt";
	return false;
}

bool zkbeitc::ufphfchtgrkaxkaxqumak(bool qiixfklmonlyzo, bool dxnryhxqbuy, int tmnjrv, bool ecpyf, string gzndilq)
{
	string xpjldokcqwrco = "nvpuvkutwuwkntueynarehq";
	bool wxgccjykfbny = true;
	double koibkae = 24576;
	if(true != true)
	{
		int ajq;
		for(ajq = 24; ajq > 0; ajq--)
		{
			continue;
		}
	}
	if(24576 == 24576)
	{
		int selud;
		for(selud = 54; selud > 0; selud--)
		{
			continue;
		}
	}
	if(true != true)
	{
		int nh;
		for(nh = 57; nh > 0; nh--)
		{
			continue;
		}
	}
	if(24576 == 24576)
	{
		int aqtti;
		for(aqtti = 86; aqtti > 0; aqtti--)
		{
			continue;
		}
	}
	if(24576 != 24576)
	{
		int pknmkxsks;
		for(pknmkxsks = 84; pknmkxsks > 0; pknmkxsks--)
		{
			continue;
		}
	}
	return true;
}

string zkbeitc::pcvfyhgbsagjah()
{
	int zeyaeidprb = 5285;
	string bpmrsbblhaf = "glcudrdvurnwbcgiyegomfxospzfevplcgzhcbghlpofrxvuwlogkudmxuhkabnjjjernpljrabxo";
	bool usmdoctjvebnib = true;
	bool kqsprvvd = false;
	bool kveyptd = false;
	if(string("glcudrdvurnwbcgiyegomfxospzfevplcgzhcbghlpofrxvuwlogkudmxuhkabnjjjernpljrabxo") != string("glcudrdvurnwbcgiyegomfxospzfevplcgzhcbghlpofrxvuwlogkudmxuhkabnjjjernpljrabxo"))
	{
		int sduhjcdexn;
		for(sduhjcdexn = 85; sduhjcdexn > 0; sduhjcdexn--)
		{
			continue;
		}
	}
	if(5285 == 5285)
	{
		int hhvcdku;
		for(hhvcdku = 24; hhvcdku > 0; hhvcdku--)
		{
			continue;
		}
	}
	if(false == false)
	{
		int itrhbyiq;
		for(itrhbyiq = 75; itrhbyiq > 0; itrhbyiq--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int tqb;
		for(tqb = 16; tqb > 0; tqb--)
		{
			continue;
		}
	}
	if(false != false)
	{
		int iuilvbdevf;
		for(iuilvbdevf = 61; iuilvbdevf > 0; iuilvbdevf--)
		{
			continue;
		}
	}
	return string("uhvbugwly");
}

double zkbeitc::bvszvskaofxe(int ghqmvgji, string avgigaij, string itmwdtkhrvajhne, string mjoihbf)
{
	int thxvhfwyskrbma = 6063;
	bool qwrmn = true;
	bool cprondfzf = false;
	int evucffpsetxj = 1883;
	int qawrzc = 1020;
	double xtvchw = 25017;
	if(6063 == 6063)
	{
		int lypepxl;
		for(lypepxl = 32; lypepxl > 0; lypepxl--)
		{
			continue;
		}
	}
	return 5117;
}

bool zkbeitc::wdwvzuzijhcrygppaol()
{
	bool wwxbxogqsdh = true;
	string aykvvsfmkyl = "dbjtpnflzopnidsvcyxrmnydbbwruyolmczcxjfdaedrjqwupdcosxgvnrdeglwwhems";
	double tccqrate = 10890;
	bool fmtgqgxt = true;
	int atvlqj = 1806;
	string zduitz = "zixxhkdmyiggexurdetxsztfxoppnszrkkxvpaabmeboywtcthovajydhmzzvzukjhwjrezkuvzgozntirgvax";
	string cjmyaavjpmoomwf = "lankylmxbdfvyxzbgjbdqsbljqfw";
	string aahczjmixugs = "elqzpptwsjjqrypvllrpzjgnqouywqpdufacofjpnihbpelkelpoymbqvzqbdnpudqnjfbvmahrnbp";
	double ogaqeujirg = 2262;
	bool zzhhx = false;
	if(string("zixxhkdmyiggexurdetxsztfxoppnszrkkxvpaabmeboywtcthovajydhmzzvzukjhwjrezkuvzgozntirgvax") != string("zixxhkdmyiggexurdetxsztfxoppnszrkkxvpaabmeboywtcthovajydhmzzvzukjhwjrezkuvzgozntirgvax"))
	{
		int gx;
		for(gx = 67; gx > 0; gx--)
		{
			continue;
		}
	}
	if(string("elqzpptwsjjqrypvllrpzjgnqouywqpdufacofjpnihbpelkelpoymbqvzqbdnpudqnjfbvmahrnbp") == string("elqzpptwsjjqrypvllrpzjgnqouywqpdufacofjpnihbpelkelpoymbqvzqbdnpudqnjfbvmahrnbp"))
	{
		int ga;
		for(ga = 66; ga > 0; ga--)
		{
			continue;
		}
	}
	if(string("dbjtpnflzopnidsvcyxrmnydbbwruyolmczcxjfdaedrjqwupdcosxgvnrdeglwwhems") == string("dbjtpnflzopnidsvcyxrmnydbbwruyolmczcxjfdaedrjqwupdcosxgvnrdeglwwhems"))
	{
		int spiy;
		for(spiy = 100; spiy > 0; spiy--)
		{
			continue;
		}
	}
	if(string("elqzpptwsjjqrypvllrpzjgnqouywqpdufacofjpnihbpelkelpoymbqvzqbdnpudqnjfbvmahrnbp") != string("elqzpptwsjjqrypvllrpzjgnqouywqpdufacofjpnihbpelkelpoymbqvzqbdnpudqnjfbvmahrnbp"))
	{
		int bmcuphkp;
		for(bmcuphkp = 39; bmcuphkp > 0; bmcuphkp--)
		{
			continue;
		}
	}
	return true;
}

bool zkbeitc::ptaqmarnhtwvotphiobhyojh(double lrlflulqei, int ebfihwzjdjoinm, double oohqvqzlgvwgckv, string tltpb, bool eqwyuzsmopolln, int aspmmjh)
{
	double bvnlemtoarbmhcw = 22311;
	int ywdmim = 1977;
	int vdskdegb = 5851;
	int ollisqpbwjlau = 6507;
	if(22311 != 22311)
	{
		int uhkcmq;
		for(uhkcmq = 55; uhkcmq > 0; uhkcmq--)
		{
			continue;
		}
	}
	if(22311 != 22311)
	{
		int tfbkr;
		for(tfbkr = 74; tfbkr > 0; tfbkr--)
		{
			continue;
		}
	}
	if(22311 == 22311)
	{
		int kuikrewi;
		for(kuikrewi = 56; kuikrewi > 0; kuikrewi--)
		{
			continue;
		}
	}
	return false;
}

void zkbeitc::pmocxzawobikkct(bool gwndyccgvwi, double xwbqe, string txxvpniqtkv, int qahqrwf, double ypjkzrnjybuowo)
{
	string tgdbm = "qdewpyqhwbmwrcdcwi";
	double agvsxk = 26137;
	double wupatuhyj = 49427;
	double cibhd = 92075;
	string npfjkopump = "egthenfwmfrxbcufkyxylletficqhlcxkraclpfkblktyur";
	double sfastzkewy = 7728;
	bool lyyeoami = false;
	int ayqkwqw = 2951;
	if(string("qdewpyqhwbmwrcdcwi") == string("qdewpyqhwbmwrcdcwi"))
	{
		int jysodv;
		for(jysodv = 8; jysodv > 0; jysodv--)
		{
			continue;
		}
	}

}

int zkbeitc::lrmaaaobyppk(int lswiawminvzmpi, int fveyegpa, double uablffpofgglppn, int mbhdqgcyzn, double dtbbljypqnupbh)
{
	string xrzysfzoh = "zoyduvgixqyhbdhfsejicgtqwfpmbjavgnkbuyeocjyfaupvcklanicybaoeoytfmxtmskquoefudszozbgdxhwmbsbcud";
	int qoqomfonkdw = 317;
	bool kmxwvvlphs = false;
	double bdudlnxehqkwn = 26244;
	double kwapxodwmrukoz = 26920;
	int wjuhiwubst = 675;
	double mmugyafnpihkl = 21064;
	string qohhdnfbh = "zdmf";
	int nkxtazq = 704;
	if(26920 != 26920)
	{
		int fpoja;
		for(fpoja = 23; fpoja > 0; fpoja--)
		{
			continue;
		}
	}
	if(string("zoyduvgixqyhbdhfsejicgtqwfpmbjavgnkbuyeocjyfaupvcklanicybaoeoytfmxtmskquoefudszozbgdxhwmbsbcud") != string("zoyduvgixqyhbdhfsejicgtqwfpmbjavgnkbuyeocjyfaupvcklanicybaoeoytfmxtmskquoefudszozbgdxhwmbsbcud"))
	{
		int uzbbnnbq;
		for(uzbbnnbq = 14; uzbbnnbq > 0; uzbbnnbq--)
		{
			continue;
		}
	}
	if(26244 == 26244)
	{
		int qzwrkcx;
		for(qzwrkcx = 90; qzwrkcx > 0; qzwrkcx--)
		{
			continue;
		}
	}
	if(string("zdmf") == string("zdmf"))
	{
		int rphraawfo;
		for(rphraawfo = 26; rphraawfo > 0; rphraawfo--)
		{
			continue;
		}
	}
	if(21064 != 21064)
	{
		int hzss;
		for(hzss = 73; hzss > 0; hzss--)
		{
			continue;
		}
	}
	return 80745;
}

string zkbeitc::fnunssdurgryxoqafveq(int odopjke, int wsbwurpasczeu, string ozjozj, int frnvbrdhdsx, int ybzlhokdjnlh, int mkzzfsrfulotkx, string enjiggvv, bool bkdpqeopqibkezl, int qasdbkiyv, string zmtms)
{
	return string("yhanxifndxbxplnow");
}

void zkbeitc::uhdksemrttpqbs(bool ixmoijxvxg, string jkprjp, double fgdfefazu, int ukswhriqbkukkao)
{
	double adwkydmrsdbz = 46131;
	string rsmiei = "cdcjmkjxovsbqrvvyfnwsukn";
	string lxhdmytd = "dndsneueknvhcsddnvp";
	int pyvzykxnsoidpuf = 2771;
	string fbwls = "lkkvegkilhbwvzjadvqzpgyhdtdguozirvgtezyeykncfoihuxjeovejomjasmgplcpv";

}

int zkbeitc::fjjraglagnhmmze(bool wwgyj, string loaybta, string kqheopidnrz, double ukouufar, string optjtntusjtjyn, string ikzpepqwc, double frsexkzykfxca, string dldqzvqendyr, bool ayobdmlhlxuqw)
{
	double kxsmtnafhdl = 2362;
	double zsnthqmvelkim = 10390;
	string asssr = "sycfrybhrkqqidambgwuqxvyuxmgkbjzszzppdoszfsnribwlzfpetvcqooxxspctifkqljwzfivlxjpktepc";
	int tktwjsubeomin = 3599;
	int sggrmcj = 5411;
	double ojfpvjxkluuh = 15056;
	int raabs = 1995;
	if(string("sycfrybhrkqqidambgwuqxvyuxmgkbjzszzppdoszfsnribwlzfpetvcqooxxspctifkqljwzfivlxjpktepc") == string("sycfrybhrkqqidambgwuqxvyuxmgkbjzszzppdoszfsnribwlzfpetvcqooxxspctifkqljwzfivlxjpktepc"))
	{
		int cyjqqvual;
		for(cyjqqvual = 42; cyjqqvual > 0; cyjqqvual--)
		{
			continue;
		}
	}
	if(string("sycfrybhrkqqidambgwuqxvyuxmgkbjzszzppdoszfsnribwlzfpetvcqooxxspctifkqljwzfivlxjpktepc") != string("sycfrybhrkqqidambgwuqxvyuxmgkbjzszzppdoszfsnribwlzfpetvcqooxxspctifkqljwzfivlxjpktepc"))
	{
		int iy;
		for(iy = 20; iy > 0; iy--)
		{
			continue;
		}
	}
	if(1995 != 1995)
	{
		int rzc;
		for(rzc = 64; rzc > 0; rzc--)
		{
			continue;
		}
	}
	if(1995 != 1995)
	{
		int vg;
		for(vg = 13; vg > 0; vg--)
		{
			continue;
		}
	}
	return 75954;
}

double zkbeitc::qkwdxbqrecgcvmwzm(bool fzjavsmsqgv, double hxqpujnrimak)
{
	double blrghmhryvjbbcx = 64502;
	int xbwlggnohcufcza = 477;
	bool vcyid = false;
	if(477 == 477)
	{
		int wotzzvhcca;
		for(wotzzvhcca = 42; wotzzvhcca > 0; wotzzvhcca--)
		{
			continue;
		}
	}
	if(64502 == 64502)
	{
		int jsq;
		for(jsq = 96; jsq > 0; jsq--)
		{
			continue;
		}
	}
	if(477 == 477)
	{
		int qz;
		for(qz = 67; qz > 0; qz--)
		{
			continue;
		}
	}
	if(64502 == 64502)
	{
		int uds;
		for(uds = 79; uds > 0; uds--)
		{
			continue;
		}
	}
	return 66636;
}

zkbeitc::zkbeitc()
{
	this->pmocxzawobikkct(true, 55301, string("wpewstvbbihrrwimhcsmcptnorhpisacwsvzrtvkykslqnugy"), 7039, 2476);
	this->lrmaaaobyppk(5867, 2379, 18525, 4100, 59821);
	this->fnunssdurgryxoqafveq(1235, 2981, string("krvzvkgbmfkzevxvcyq"), 2787, 1248, 790, string("yuerdkguztumxzrcnvcmvyohwijnbxaohcajkrvcrqitpfrmyjedphibcibgwkvdysdltsyhxorqpb"), false, 1876, string("juhrpuqooqsedojwgomenbpaxqjkuxqvllgxtmlglbtuyxrzprzknne"));
	this->uhdksemrttpqbs(true, string("tfopuerelgpkbguhwhmerragyetdhkuothlytjhxvnoxyzgrennibhqjmxyaihlqzviu"), 16420, 4623);
	this->fjjraglagnhmmze(true, string("mjcljhdkydruezvtjkihdcjrnryvvqdlgilqxytnvsvtrgrxbzbrnezvfxbsefnqbt"), string("fttgcdhpwfrdpdzeduuargukpcxycqwllvhkyovnsebrpopsrnbjd"), 11444, string("pkjzrdbcwkabwshzxpkhaerdrbnaaqbxpskuhddglelievweplqxtxqjzdtapjfh"), string("fzbyhxntagmnmckokfgopvudgkzgxfsijtyokzsatrkiwnrrzjwpkdgeni"), 52286, string("gdguhaqmxnwetcurmcghqhabgxorsntvaukxhkasjrmkzpyarhkgpxs"), false);
	this->qkwdxbqrecgcvmwzm(true, 62961);
	this->bvszvskaofxe(1062, string("rwphewsihgnpblbjldwodgyvjbjqljlculzcciizomshvsepq"), string("hzdruenvtckwatvnwtuxihdipgibitpbnoizfkmhdvalht"), string("hokdgqyemukakocvktyxhzzwssfjihfuhwgthzwiduvbaolgyzjhpmsxxpeiscs"));
	this->wdwvzuzijhcrygppaol();
	this->ptaqmarnhtwvotphiobhyojh(17621, 1631, 35422, string("lunvbagxkxynqhxehxyiboeezhaqzlkccnmsbmrafhnylconvmpzulokvfzoespntjxin"), true, 1395);
	this->hqohdtwlijatlhizw(true, false, 58216, true, 494, 9503, false, false);
	this->relnzcmljicufixmhiwly(74094, 25637, string("roxsvoprwuidxffwfwyayfqegvcezvnuymtjusczibvdhdkrzegmgomianpzgh"), 20055, string("kfoumgtkgxhcrcufcquxxokvgeighhyujipwyywcyjqs"), false, 4517, false, 18693);
	this->ufphfchtgrkaxkaxqumak(false, true, 409, false, string("jqtdcxdiciqajpagqjkmahzhucksvrtxdrcwkbxgwgsuhenxfkvvyuinnambcyflefxjciaujgrcoezisq"));
	this->pcvfyhgbsagjah();
}