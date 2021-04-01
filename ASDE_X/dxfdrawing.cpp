#include "dxfdrawing.h"

DxfDrawing* aircraftBlip = new DxfDrawing();
DxfDrawing* unknownBlip = new DxfDrawing();

void loadUnknownBlip() {
	unknownBlip->setMinX(-9.46875);
	unknownBlip->setMaxX(9.46875);
	unknownBlip->setMinY(-15.307692500000002);
	unknownBlip->setMaxY(15.307692500000002);
	unknownBlip->addCoordinates(-2.03125, -13.415865499999995);
	unknownBlip->addCoordinates(-0.9615379999999902, -14.846153499999986);
	unknownBlip->addCoordinates(-0.1153850000000034, -15.307692500000002);
	unknownBlip->addCoordinates(1.03125, -14.165865499999995);
	unknownBlip->addCoordinates(1.96875, -13.103365499999995);
	unknownBlip->addCoordinates(2.65625, -12.228365499999995);
	unknownBlip->addCoordinates(3.46875, -11.290865499999995);
	unknownBlip->addCoordinates(4.53125, -9.978365499999995);
	unknownBlip->addCoordinates(6.03125, -8.103365499999995);
	unknownBlip->addCoordinates(7.40625, -6.228365499999995);
	unknownBlip->addCoordinates(8.40625, -4.790865499999995);
	unknownBlip->addCoordinates(9.46875, -3.3533654999999953);
	unknownBlip->addCoordinates(7.21875, 1.1466345000000047);
	unknownBlip->addCoordinates(6.46875, 2.8341345000000047);
	unknownBlip->addCoordinates(4.78125, 5.771634500000005);
	unknownBlip->addCoordinates(3.84375, 7.646634500000005);
	unknownBlip->addCoordinates(2.53125, 10.084134500000005);
	unknownBlip->addCoordinates(1.34375, 12.334134500000005);
	unknownBlip->addCoordinates(0.46875, 14.021634500000005);
	unknownBlip->addCoordinates(-0.19230799999999704, 15.307692500000002);
	unknownBlip->addCoordinates(-1.59375, 12.584134500000005);
	unknownBlip->addCoordinates(-2.78125, 10.459134500000005);
	unknownBlip->addCoordinates(-3.84375, 8.396634500000005);
	unknownBlip->addCoordinates(-4.90625, 6.334134500000005);
	unknownBlip->addCoordinates(-6.40625, 2.8341345000000047);
	unknownBlip->addCoordinates(-7.09375, 1.3341345000000047);
	unknownBlip->addCoordinates(-8.03125, -0.5408654999999953);
	unknownBlip->addCoordinates(-8.84375, -2.1658654999999953);
	unknownBlip->addCoordinates(-9.46875, -3.6033654999999953);
	unknownBlip->addCoordinates(-6.59375, -7.415865499999995);
	unknownBlip->addCoordinates(-5.09375, -9.540865499999995);
	unknownBlip->addCoordinates(-3.96875, -10.978365499999995);
	unknownBlip->addCoordinates(-3.03125, -12.290865499999995);
	unknownBlip->addCoordinates(-2.15625, -13.290865499999995);
}

void loadAircraftBlip2() {
	aircraftBlip->setMinX(-17.70422160212365);
	aircraftBlip->setMaxX(17.69419252735628);
	aircraftBlip->setMinY(-17.460185);
	aircraftBlip->setMaxY(17.460185);

	aircraftBlip->addCoordinates(-3.011363999999999, -16.910185);
	aircraftBlip->addCoordinates(-1.859847999999999, -16.879882);
	aircraftBlip->addCoordinates(-1.859847999999999, -16.879882);
	aircraftBlip->addCoordinates(-0.890152, -15.273821);
	aircraftBlip->addCoordinates(-0.890152, -15.273821);
	aircraftBlip->addCoordinates(-0.07197, -14.122306);
	aircraftBlip->addCoordinates(-0.07197, -14.122306);
	aircraftBlip->addCoordinates(1.958333, -16.879882);
	aircraftBlip->addCoordinates(1.958333, -16.879882);
	aircraftBlip->addCoordinates(10.275, -17.460185);
	aircraftBlip->addCoordinates(10.275, -17.460185);
	aircraftBlip->addCoordinates(10.175, -15.360185);
	aircraftBlip->addCoordinates(10.175, -15.360185);
	aircraftBlip->addCoordinates(3.732143, -12.445899);
	aircraftBlip->addCoordinates(3.732143, -12.445899);
	aircraftBlip->addCoordinates(3.732143, -12.0263458496438);
	aircraftBlip->addCoordinates(3.732143, -12.0263458496438);
	aircraftBlip->addCoordinates(3.732143, -11.63378309932523);
	aircraftBlip->addCoordinates(3.732143, -11.63378309932523);
	aircraftBlip->addCoordinates(3.732143, -11.17354001173225);
	aircraftBlip->addCoordinates(3.732143, -11.17354001173225);
	aircraftBlip->addCoordinates(3.732143, -10.72682911602533);
	aircraftBlip->addCoordinates(3.732143, -10.72682911602533);
	aircraftBlip->addCoordinates(3.748889182370476, -10.15828973765747);
	aircraftBlip->addCoordinates(3.748889182370476, -10.15828973765747);
	aircraftBlip->addCoordinates(3.789601781632882, -9.194901023802231);
	aircraftBlip->addCoordinates(3.789601781632882, -9.194901023802231);
	aircraftBlip->addCoordinates(3.820750890203416, -8.36214262028371);
	aircraftBlip->addCoordinates(3.820750890203416, -8.36214262028371);
	aircraftBlip->addCoordinates(3.828372750498601, -7.299619046039879);
	aircraftBlip->addCoordinates(3.828372750498601, -7.299619046039879);
	aircraftBlip->addCoordinates(3.88632887685344, -6.381756339350303);
	aircraftBlip->addCoordinates(3.88632887685344, -6.381756339350303);
	aircraftBlip->addCoordinates(3.929214252986639, -5.298734159630811);
	aircraftBlip->addCoordinates(3.929214252986639, -5.298734159630811);
	aircraftBlip->addCoordinates(3.954212675746788, -4.65077212520498);
	aircraftBlip->addCoordinates(3.954212675746788, -4.65077212520498);
	aircraftBlip->addCoordinates(3.954212675746788, -4.65077212520498);
	aircraftBlip->addCoordinates(3.954212675746788, -4.65077212520498);
	aircraftBlip->addCoordinates(3.987020636267594, -3.835878312188114);
	aircraftBlip->addCoordinates(3.987020636267594, -3.835878312188114);
	aircraftBlip->addCoordinates(5.06924408116356, -3.772321697820046);
	aircraftBlip->addCoordinates(5.06924408116356, -3.772321697820046);
	aircraftBlip->addCoordinates(6.120667760302239, -3.72665778312276);
	aircraftBlip->addCoordinates(6.120667760302239, -3.72665778312276);
	aircraftBlip->addCoordinates(7.960670756116087, -3.712378942418155);
	aircraftBlip->addCoordinates(7.960670756116087, -3.712378942418155);
	aircraftBlip->addCoordinates(9.858848479168486, -3.708567918045375);
	aircraftBlip->addCoordinates(9.858848479168486, -3.708567918045375);
	aircraftBlip->addCoordinates(12.10826924746639, -3.7556028807941);
	aircraftBlip->addCoordinates(12.10826924746639, -3.7556028807941);
	aircraftBlip->addCoordinates(14.15974186002859, -3.767545416660766);
	aircraftBlip->addCoordinates(14.15974186002859, -3.767545416660766);
	aircraftBlip->addCoordinates(15.36635728298643, -3.815614586708036);
	aircraftBlip->addCoordinates(15.36635728298643, -3.815614586708036);
	aircraftBlip->addCoordinates(16.017543134476, -3.836978698420296);
	aircraftBlip->addCoordinates(16.017543134476, -3.836978698420296);
	aircraftBlip->addCoordinates(16.74784597906978, -3.812375463972401);
	aircraftBlip->addCoordinates(16.74784597906978, -3.812375463972401);
	aircraftBlip->addCoordinates(17.20926131435021, -3.787107722565451);
	aircraftBlip->addCoordinates(17.20926131435021, -3.787107722565451);
	aircraftBlip->addCoordinates(17.54548290500088, -3.760479199177325);
	aircraftBlip->addCoordinates(17.54548290500088, -3.760479199177325);
	aircraftBlip->addCoordinates(17.75067833417665, -3.660504588476214);
	aircraftBlip->addCoordinates(17.75067833417665, -3.660504588476214);
	aircraftBlip->addCoordinates(17.8513976340364, -3.432495697540502);
	aircraftBlip->addCoordinates(17.8513976340364, -3.432495697540502);
	aircraftBlip->addCoordinates(17.89864564908453, -3.204131539149898);
	aircraftBlip->addCoordinates(17.89864564908453, -3.204131539149898);
	aircraftBlip->addCoordinates(17.93398202515595, -3.026740642692857);
	aircraftBlip->addCoordinates(17.93398202515595, -3.026740642692857);
	aircraftBlip->addCoordinates(17.95695807933953, -2.886942442690269);
	aircraftBlip->addCoordinates(17.95695807933953, -2.886942442690269);
	aircraftBlip->addCoordinates(17.98784567858706, -2.692067233359352);
	aircraftBlip->addCoordinates(17.98784567858706, -2.692067233359352);
	aircraftBlip->addCoordinates(17.9890503110978, -2.406874962648544);
	aircraftBlip->addCoordinates(17.9890503110978, -2.406874962648544);
	aircraftBlip->addCoordinates(17.98357737654851, -2.153786538268491);
	aircraftBlip->addCoordinates(17.98357737654851, -2.153786538268491);
	aircraftBlip->addCoordinates(17.96558606514719, -1.894371417095612);
	aircraftBlip->addCoordinates(17.96558606514719, -1.894371417095612);
	aircraftBlip->addCoordinates(17.9572665423205, -1.692839665856865);
	aircraftBlip->addCoordinates(17.9572665423205, -1.692839665856865);
	aircraftBlip->addCoordinates(17.9327040289113, -1.344107263498841);
	aircraftBlip->addCoordinates(17.9327040289113, -1.344107263498841);
	aircraftBlip->addCoordinates(17.88067992520905, -0.9115439178564201);
	aircraftBlip->addCoordinates(17.88067992520905, -0.9115439178564201);
	aircraftBlip->addCoordinates(17.76426607054031, -0.6045242377891215);
	aircraftBlip->addCoordinates(17.76426607054031, -0.6045242377891215);
	aircraftBlip->addCoordinates(17.589286, -0.1203593543560828);
	aircraftBlip->addCoordinates(17.589286, -0.1203593543560828);
	aircraftBlip->addCoordinates(17.15335068702916, 0.1575509961472505);
	aircraftBlip->addCoordinates(17.15335068702916, 0.1575509961472505);
	aircraftBlip->addCoordinates(16.58195659896955, 0.3723318366651256);
	aircraftBlip->addCoordinates(16.58195659896955, 0.3723318366651256);
	aircraftBlip->addCoordinates(16.15194710524338, 0.4709711115107212);
	aircraftBlip->addCoordinates(16.15194710524338, 0.4709711115107212);
	aircraftBlip->addCoordinates(15.95755291913701, 0.5089500832742715);
	aircraftBlip->addCoordinates(15.95755291913701, 0.5089500832742715);
	aircraftBlip->addCoordinates(15.61000107819564, 0.5680524945249772);
	aircraftBlip->addCoordinates(15.61000107819564, 0.5680524945249772);
	aircraftBlip->addCoordinates(15.29779923827845, 0.6254695874356458);
	aircraftBlip->addCoordinates(15.29779923827845, 0.6254695874356458);
	aircraftBlip->addCoordinates(14.95024739733708, 0.7115952268015917);
	aircraftBlip->addCoordinates(14.95024739733708, 0.7115952268015917);
	aircraftBlip->addCoordinates(14.75922513151613, 0.7812100314055215);
	aircraftBlip->addCoordinates(14.75922513151613, 0.7812100314055215);
	aircraftBlip->addCoordinates(14.60640930977206, 0.8134460019963398);
	aircraftBlip->addCoordinates(14.60640930977206, 0.8134460019963398);
	aircraftBlip->addCoordinates(14.375, 0.873325303896422);
	aircraftBlip->addCoordinates(14.375, 0.873325303896422);
	aircraftBlip->addCoordinates(13.85136164184405, 0.9730708992133259);
	aircraftBlip->addCoordinates(13.85136164184405, 0.9730708992133259);
	aircraftBlip->addCoordinates(13.17840179233826, 1.153360617930388);
	aircraftBlip->addCoordinates(13.17840179233826, 1.153360617930388);
	aircraftBlip->addCoordinates(12.79680978040051, 1.287861374038016);
	aircraftBlip->addCoordinates(12.79680978040051, 1.287861374038016);
	aircraftBlip->addCoordinates(12.3087825212624, 1.468732154721919);
	aircraftBlip->addCoordinates(12.3087825212624, 1.468732154721919);
	aircraftBlip->addCoordinates(11.78748557937979, 1.617087504864457);
	aircraftBlip->addCoordinates(11.78748557937979, 1.617087504864457);
	aircraftBlip->addCoordinates(11.38926643095328, 1.739459234016862);
	aircraftBlip->addCoordinates(11.38926643095328, 1.739459234016862);
	aircraftBlip->addCoordinates(10.77997973063611, 1.873152067824094);
	aircraftBlip->addCoordinates(10.77997973063611, 1.873152067824094);
	aircraftBlip->addCoordinates(10.53152808873164, 1.936152563646828);
	aircraftBlip->addCoordinates(10.53152808873164, 1.936152563646828);
	aircraftBlip->addCoordinates(10.15885833075845, 2.041830178437976);
	aircraftBlip->addCoordinates(10.15885833075845, 2.041830178437976);
	aircraftBlip->addCoordinates(9.945895377764828, 2.096486548869222);
	aircraftBlip->addCoordinates(9.945895377764828, 2.096486548869222);
	aircraftBlip->addCoordinates(9.691541795846206, 2.168617848551235);
	aircraftBlip->addCoordinates(9.691541795846206, 2.168617848551235);
	aircraftBlip->addCoordinates(9.291667, 2.291877476527972);
	aircraftBlip->addCoordinates(9.291667, 2.291877476527972);
	aircraftBlip->addCoordinates(8.84882797454884, 2.484620036213897);
	aircraftBlip->addCoordinates(8.84882797454884, 2.484620036213897);
	aircraftBlip->addCoordinates(8.273735542876238, 2.675806249420105);
	aircraftBlip->addCoordinates(8.273735542876238, 2.675806249420105);
	aircraftBlip->addCoordinates(7.689058237343488, 2.907670328432743);
	aircraftBlip->addCoordinates(7.689058237343488, 2.907670328432743);
	aircraftBlip->addCoordinates(7.20981454428329, 3.082585395316385);
	aircraftBlip->addCoordinates(7.20981454428329, 3.082585395316385);
	aircraftBlip->addCoordinates(6.56762799558237, 3.336281498171203);
	aircraftBlip->addCoordinates(6.56762799558237, 3.336281498171203);
	aircraftBlip->addCoordinates(6.13629326206319, 3.511900181314132);
	aircraftBlip->addCoordinates(6.13629326206319, 3.511900181314132);
	aircraftBlip->addCoordinates(5.589955451974674, 3.731106566443756);
	aircraftBlip->addCoordinates(5.589955451974674, 3.731106566443756);
	aircraftBlip->addCoordinates(5.120296632774625, 3.9537466379453);
	aircraftBlip->addCoordinates(5.120296632774625, 3.9537466379453);
	aircraftBlip->addCoordinates(4.735853814303482, 4.142535563723114);
	aircraftBlip->addCoordinates(4.735853814303482, 4.142535563723114);
	aircraftBlip->addCoordinates(4.549750081792808, 4.206804087493352);
	aircraftBlip->addCoordinates(4.549750081792808, 4.206804087493352);
	aircraftBlip->addCoordinates(4.166666999999999, 4.40321311068898);
	aircraftBlip->addCoordinates(4.166666999999999, 4.40321311068898);
	aircraftBlip->addCoordinates(3.995369999999999, 11.867593);
	aircraftBlip->addCoordinates(3.995369999999999, 11.867593);
	aircraftBlip->addCoordinates(3.921295999999999, 14.237963);
	aircraftBlip->addCoordinates(3.921295999999999, 14.237963);
	aircraftBlip->addCoordinates(2.847222, 16.534259);
	aircraftBlip->addCoordinates(2.847222, 16.534259);
	aircraftBlip->addCoordinates(1.513888999999999, 17.460185);
	aircraftBlip->addCoordinates(1.513888999999999, 17.460185);
	aircraftBlip->addCoordinates(-1.449073999999999, 17.460185);
	aircraftBlip->addCoordinates(-1.449073999999999, 17.460185);
	aircraftBlip->addCoordinates(-2.782407, 16.423148);
	aircraftBlip->addCoordinates(-2.782407, 16.423148);
	aircraftBlip->addCoordinates(-3.745369999999999, 14.015741);
	aircraftBlip->addCoordinates(-3.745369999999999, 14.015741);
	aircraftBlip->addCoordinates(-3.893519, 12.237963);
	aircraftBlip->addCoordinates(-3.893519, 12.237963);
	aircraftBlip->addCoordinates(-4.060442466863783, 4.562386444224194);
	aircraftBlip->addCoordinates(-4.060442466863783, 4.562386444224194);
	aircraftBlip->addCoordinates(-4.726913068643625, 4.194986684116443);
	aircraftBlip->addCoordinates(-4.726913068643625, 4.194986684116443);
	aircraftBlip->addCoordinates(-5.31784673878792, 3.951712216170392);
	aircraftBlip->addCoordinates(-5.31784673878792, 3.951712216170392);
	aircraftBlip->addCoordinates(-5.799139930599267, 3.793525556742011);
	aircraftBlip->addCoordinates(-5.799139930599267, 3.793525556742011);
	aircraftBlip->addCoordinates(-6.249968016907587, 3.64577092251541);
	aircraftBlip->addCoordinates(-6.249968016907587, 3.64577092251541);
	aircraftBlip->addCoordinates(-6.816538848506752, 3.407866479351852);
	aircraftBlip->addCoordinates(-6.816538848506752, 3.407866479351852);
	aircraftBlip->addCoordinates(-7.139435065368161, 3.270132357591236);
	aircraftBlip->addCoordinates(-7.139435065368161, 3.270132357591236);
	aircraftBlip->addCoordinates(-7.756858121632831, 3.032228026697168);
	aircraftBlip->addCoordinates(-7.756858121632831, 3.032228026697168);
	aircraftBlip->addCoordinates(-8.302718229004311, 2.838148092130552);
	aircraftBlip->addCoordinates(-8.302718229004311, 2.838148092130552);
	aircraftBlip->addCoordinates(-9.125, 2.587722457027425);
	aircraftBlip->addCoordinates(-9.125, 2.587722457027425);
	aircraftBlip->addCoordinates(-9.566657977949944, 2.449988335266809);
	aircraftBlip->addCoordinates(-9.566657977949944, 2.449988335266809);
	aircraftBlip->addCoordinates(-9.905272155161583, 2.33103611368503);
	aircraftBlip->addCoordinates(-9.905272155161583, 2.33103611368503);
	aircraftBlip->addCoordinates(-10.29313594147424, 2.230865904551578);
	aircraftBlip->addCoordinates(-10.29313594147424, 2.230865904551578);
	aircraftBlip->addCoordinates(-10.63789861494297, 2.136956179118413);
	aircraftBlip->addCoordinates(-10.63789861494297, 2.136956179118413);
	aircraftBlip->addCoordinates(-11.09965222466962, 1.998967205610227);
	aircraftBlip->addCoordinates(-11.09965222466962, 1.998967205610227);
	aircraftBlip->addCoordinates(-11.42025238653878, 1.939493677017665);
	aircraftBlip->addCoordinates(-11.42025238653878, 1.939493677017665);
	aircraftBlip->addCoordinates(-11.82830296506836, 1.810634458625017);
	aircraftBlip->addCoordinates(-11.82830296506836, 1.810634458625017);
	aircraftBlip->addCoordinates(-12.47973537279904, 1.661950805547832);
	aircraftBlip->addCoordinates(-12.47973537279904, 1.661950805547832);
	aircraftBlip->addCoordinates(-13.05590648802354, 1.467067582763661);
	aircraftBlip->addCoordinates(-13.05590648802354, 1.467067582763661);
	aircraftBlip->addCoordinates(-13.410714, 1.387207935216494);
	aircraftBlip->addCoordinates(-13.410714, 1.387207935216494);
	aircraftBlip->addCoordinates(-13.78552804361971, 1.316428307399746);
	aircraftBlip->addCoordinates(-13.78552804361971, 1.316428307399746);
	aircraftBlip->addCoordinates(-14.10640558125851, 1.227061230164878);
	aircraftBlip->addCoordinates(-14.10640558125851, 1.227061230164878);
	aircraftBlip->addCoordinates(-14.41162679760782, 1.131452305965439);
	aircraftBlip->addCoordinates(-14.41162679760782, 1.131452305965439);
	aircraftBlip->addCoordinates(-14.79814993436048, 1.017536606078664);
	aircraftBlip->addCoordinates(-14.79814993436048, 1.017536606078664);
	aircraftBlip->addCoordinates(-15.17461049453595, 0.9094627369553052);
	aircraftBlip->addCoordinates(-15.17461049453595, 0.9094627369553052);
	aircraftBlip->addCoordinates(-15.51277778856274, 0.8265223022693533);
	aircraftBlip->addCoordinates(-15.51277778856274, 0.8265223022693533);
	aircraftBlip->addCoordinates(-15.88923834873822, 0.7063404780451493);
	aircraftBlip->addCoordinates(-15.88923834873822, 0.7063404780451493);
	aircraftBlip->addCoordinates(-16.36140895964127, 0.5746243841861088);
	aircraftBlip->addCoordinates(-16.36140895964127, 0.5746243841861088);
	aircraftBlip->addCoordinates(-16.79530171415717, 0.4587141969348068);
	aircraftBlip->addCoordinates(-16.79530171415717, 0.4587141969348068);
	aircraftBlip->addCoordinates(-17.11433007999221, 0.3913182600384033);
	aircraftBlip->addCoordinates(-17.11433007999221, 0.3913182600384033);
	aircraftBlip->addCoordinates(-17.45759463855182, 0.2585056356477935);
	aircraftBlip->addCoordinates(-17.45759463855182, 0.2585056356477935);
	aircraftBlip->addCoordinates(-17.73196695766933, -0.0839567014921841);
	aircraftBlip->addCoordinates(-17.73196695766933, -0.0839567014921841);
	aircraftBlip->addCoordinates(-17.87960537340063, -0.4791049830693055);
	aircraftBlip->addCoordinates(-17.87960537340063, -0.4791049830693055);
	aircraftBlip->addCoordinates(-17.94614403948173, -1.021869932848858);
	aircraftBlip->addCoordinates(-17.94614403948173, -1.021869932848858);
	aircraftBlip->addCoordinates(-17.97592558266251, -1.569321973643667);
	aircraftBlip->addCoordinates(-17.97592558266251, -1.569321973643667);
	aircraftBlip->addCoordinates(-17.98585276372273, -1.903652992885156);
	aircraftBlip->addCoordinates(-17.98585276372273, -1.903652992885156);
	aircraftBlip->addCoordinates(-17.98909656066371, -2.21759307922548);
	aircraftBlip->addCoordinates(-17.98909656066371, -2.21759307922548);
	aircraftBlip->addCoordinates(-17.98920155351328, -2.476016461395914);
	aircraftBlip->addCoordinates(-17.98920155351328, -2.476016461395914);
	aircraftBlip->addCoordinates(-17.88031084442662, -2.919592260197944);
	aircraftBlip->addCoordinates(-17.88031084442662, -2.919592260197944);
	aircraftBlip->addCoordinates(-17.7745460538008, -3.182480346073134);
	aircraftBlip->addCoordinates(-17.7745460538008, -3.182480346073134);
	aircraftBlip->addCoordinates(-17.56036881127784, -3.338783725101983);
	aircraftBlip->addCoordinates(-17.56036881127784, -3.338783725101983);
	aircraftBlip->addCoordinates(-17.28348563637428, -3.445898999999999);
	aircraftBlip->addCoordinates(-17.28348563637428, -3.445898999999999);
	aircraftBlip->addCoordinates(-16.36764991448581, -3.445898999999999);
	aircraftBlip->addCoordinates(-16.36764991448581, -3.445898999999999);
	aircraftBlip->addCoordinates(-15.4654698098584, -3.482119112819134);
	aircraftBlip->addCoordinates(-15.4654698098584, -3.482119112819134);
	aircraftBlip->addCoordinates(-14.33570288520605, -3.482119112819134);
	aircraftBlip->addCoordinates(-14.33570288520605, -3.482119112819134);
	aircraftBlip->addCoordinates(-12.79598998092297, -3.504456812358753);
	aircraftBlip->addCoordinates(-12.79598998092297, -3.504456812358753);
	aircraftBlip->addCoordinates(-11.71818478401837, -3.554542590542439);
	aircraftBlip->addCoordinates(-11.71818478401837, -3.554542590542439);
	aircraftBlip->addCoordinates(-10.45219553632341, -3.571237849937006);
	aircraftBlip->addCoordinates(-10.45219553632341, -3.571237849937006);
	aircraftBlip->addCoordinates(-9.39149517862461, -3.593498195796414);
	aircraftBlip->addCoordinates(-9.39149517862461, -3.593498195796414);
	aircraftBlip->addCoordinates(-8.26237546410266, -3.599708382764148);
	aircraftBlip->addCoordinates(-8.26237546410266, -3.599708382764148);
	aircraftBlip->addCoordinates(-7.098121485267256, -3.612806527624741);
	aircraftBlip->addCoordinates(-7.098121485267256, -3.612806527624741);
	aircraftBlip->addCoordinates(-6.00422849365168, -3.673577442470573);
	aircraftBlip->addCoordinates(-6.00422849365168, -3.673577442470573);
	aircraftBlip->addCoordinates(-4.38234054393429, -3.762600183041726);
	aircraftBlip->addCoordinates(-4.38234054393429, -3.762600183041726);
	aircraftBlip->addCoordinates(-4.351424987081331, -4.332084120912549);
	aircraftBlip->addCoordinates(-4.351424987081331, -4.332084120912549);
	aircraftBlip->addCoordinates(-4.323672983117717, -4.779335190448591);
	aircraftBlip->addCoordinates(-4.323672983117717, -4.779335190448591);
	aircraftBlip->addCoordinates(-4.310186792392414, -5.228164148160018);
	aircraftBlip->addCoordinates(-4.310186792392414, -5.228164148160018);
	aircraftBlip->addCoordinates(-4.2600896462136, -5.75027131632315);
	aircraftBlip->addCoordinates(-4.2600896462136, -5.75027131632315);
	aircraftBlip->addCoordinates(-4.236327788506969, -6.2174198266475);
	aircraftBlip->addCoordinates(-4.236327788506969, -6.2174198266475);
	aircraftBlip->addCoordinates(-4.213254190184087, -6.712047747196564);
	aircraftBlip->addCoordinates(-4.213254190184087, -6.712047747196564);
	aircraftBlip->addCoordinates(-4.194860110756053, -7.170058805053486);
	aircraftBlip->addCoordinates(-4.194860110756053, -7.170058805053486);
	aircraftBlip->addCoordinates(-4.157969133226288, -8.13428722532717);
	aircraftBlip->addCoordinates(-4.157969133226288, -8.13428722532717);
	aircraftBlip->addCoordinates(-4.148322620305407, -8.747880082649317);
	aircraftBlip->addCoordinates(-4.148322620305407, -8.747880082649317);
	aircraftBlip->addCoordinates(-4.142066255693862, -9.43160780369817);
	aircraftBlip->addCoordinates(-4.142066255693862, -9.43160780369817);
	aircraftBlip->addCoordinates(-4.131341059219266, -9.94001774327262);
	aircraftBlip->addCoordinates(-4.131341059219266, -9.94001774327262);
	aircraftBlip->addCoordinates(-4.136688247692291, -10.58867803245812);
	aircraftBlip->addCoordinates(-4.136688247692291, -10.58867803245812);
	aircraftBlip->addCoordinates(-4.135455466488565, -11.06202699944879);
	aircraftBlip->addCoordinates(-4.135455466488565, -11.06202699944879);
	aircraftBlip->addCoordinates(-4.149909826104703, -11.74574826121807);
	aircraftBlip->addCoordinates(-4.149909826104703, -11.74574826121807);
	aircraftBlip->addCoordinates(-4.174999999999999, -12.443518);
	aircraftBlip->addCoordinates(-4.174999999999999, -12.443518);
	aircraftBlip->addCoordinates(-10.125, -15.374471);
	aircraftBlip->addCoordinates(-10.125, -15.374471);
	aircraftBlip->addCoordinates(-10.198529, -17.204303);
	aircraftBlip->addCoordinates(-10.198529, -17.204303);
	aircraftBlip->addCoordinates(-3.465908999999999, -16.940488);
	aircraftBlip->addCoordinates(-3.465908999999999, -16.940488);
	aircraftBlip->addCoordinates(-3.011363999999999, -16.910185);
}

//-3.011364, -16.910185
//-1.859848, -16.879882
//-0.890152, -15.273821
//-0.07197, -14.122306
//1.958333, -16.879882
//10.275, -17.460185
//10.175, -15.360185
//3.732143, -12.445899
//3.946429, -5.874471
//4.017857, -2.803042
//17.625, -3.799074
//17.589286, -1.160185
//14.375, -0.017328
//9.291667, 1.756482
//4.166667, 3.464815
//3.99537, 11.867593
//3.921296, 14.237963
//2.847222, 16.534259
//1.513889, 17.460185
//-1.449074, 17.460185
//-2.782407, 16.423148
//-3.74537, 14.015741
//-3.893519, 12.237963
//-4.083333, 3.464815
//-9.125, 1.911244
//-13.410714, 0.482672
//-17.625, -0.945899
//-17.625, -3.445899
//-4.410714, -2.803042
//-4.375, -6.310185
//-4.175, -12.443518
//-10.125, -15.374471
//-10.198529, -17.204303
//-3.465909, -16.940488

double DxfDrawing::getMinX() {
	return DxfDrawing::minX;
}

void DxfDrawing::setMinX(double value) {
	DxfDrawing::minX = value;
}

double DxfDrawing::getMinY() {
	return DxfDrawing::minY;
}

void DxfDrawing::setMinY(double value) {
	DxfDrawing::minY = value;
}

double DxfDrawing::getMaxX() {
	return DxfDrawing::maxX;
}

void DxfDrawing::setMaxX(double value) {
	DxfDrawing::maxX = value;
}

double DxfDrawing::getMaxY() {
	return DxfDrawing::maxY;
}

void DxfDrawing::setMaxY(double value) {
	DxfDrawing::maxY = value;
}

void DxfDrawing::addCoordinates(double x, double y) {
	double* value = new double[3];
	value[0] = x;
	value[1] = y;
	value[2] = 0;
	DxfDrawing::coordinates.push_back(value);
	DxfDrawing::formated_coordinates.push_back(NULL);
}

std::vector<double*> DxfDrawing::getCoordinates() {
	return DxfDrawing::coordinates;
}

std::vector<double*> DxfDrawing::getFCoordinates() {
	return DxfDrawing::formated_coordinates;
}

double* DxfDrawing::formatCoordinates(size_t i, double aircraft_size, double* coords, double latitude, double longitude) {
	double* object = DxfDrawing::formated_coordinates[i];
	double x = coords[0];
	double y = coords[1];
	double valueX = (longitude + (x - DxfDrawing::minX));
	double valueY = (latitude + (y - DxfDrawing::minY));
	double vMaxX = (longitude + (maxX - minX));
	double vMaxY = (latitude + (maxY - minY));
	double offsetX = ((longitude + vMaxX) / 2.0);
	double offsetY = ((latitude + vMaxY) / 2.0);
	double x1 = longitude + ((valueX - offsetX) * aircraft_size);
	double y1 = latitude + ((valueY - offsetY) * aircraft_size);
	if (object == NULL) {
		DxfDrawing::formated_coordinates[i] = new double[2];
		object = DxfDrawing::formated_coordinates[i];
	}
	object[0] = x1;
	object[1] = y1;
	return object;
}