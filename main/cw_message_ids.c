
const char *name_from_id(int id) {
	if (id==1) return "INIT";
	if (id==7) return "bottom_forward";
	if (id==8) return "bottom_reverse";
	if (id==9) return "falling";
	if (id==13) return "fast";
	if (id==6) return "idle";
	if (id==2) return "off";
	if (id==3) return "on";
	if (id==-102) return "property_change";
	if (id==10) return "rising";
	if (id==14) return "slow";
	if (id==5) return "stable";
	if (id==11) return "stopped";
	if (id==18) return "toggle_speed";
	if (id==12) return "top";
	if (id==-100) return "turnOff";
	if (id==-101) return "turnOn";
	if (id==4) return "unstable";
	return "*unknown*";
}
