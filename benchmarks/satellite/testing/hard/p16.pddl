;; satellites=75, instruments=125, modes=8, directions=70, out_folder=testing/hard, instance_id=16, seed=1022

(define (problem satellite-16)
 (:domain satellite)
 (:objects 
    sat1 sat2 sat3 sat4 sat5 sat6 sat7 sat8 sat9 sat10 sat11 sat12 sat13 sat14 sat15 sat16 sat17 sat18 sat19 sat20 sat21 sat22 sat23 sat24 sat25 sat26 sat27 sat28 sat29 sat30 sat31 sat32 sat33 sat34 sat35 sat36 sat37 sat38 sat39 sat40 sat41 sat42 sat43 sat44 sat45 sat46 sat47 sat48 sat49 sat50 sat51 sat52 sat53 sat54 sat55 sat56 sat57 sat58 sat59 sat60 sat61 sat62 sat63 sat64 sat65 sat66 sat67 sat68 sat69 sat70 sat71 sat72 sat73 sat74 sat75 - satellite
    ins1 ins2 ins3 ins4 ins5 ins6 ins7 ins8 ins9 ins10 ins11 ins12 ins13 ins14 ins15 ins16 ins17 ins18 ins19 ins20 ins21 ins22 ins23 ins24 ins25 ins26 ins27 ins28 ins29 ins30 ins31 ins32 ins33 ins34 ins35 ins36 ins37 ins38 ins39 ins40 ins41 ins42 ins43 ins44 ins45 ins46 ins47 ins48 ins49 ins50 ins51 ins52 ins53 ins54 ins55 ins56 ins57 ins58 ins59 ins60 ins61 ins62 ins63 ins64 ins65 ins66 ins67 ins68 ins69 ins70 ins71 ins72 ins73 ins74 ins75 ins76 ins77 ins78 ins79 ins80 ins81 ins82 ins83 ins84 ins85 ins86 ins87 ins88 ins89 ins90 ins91 ins92 ins93 ins94 ins95 ins96 ins97 ins98 ins99 ins100 ins101 ins102 ins103 ins104 ins105 ins106 ins107 ins108 ins109 ins110 ins111 ins112 ins113 ins114 ins115 ins116 ins117 ins118 ins119 ins120 ins121 ins122 ins123 ins124 ins125 - instrument
    mod1 mod2 mod3 mod4 mod5 mod6 mod7 mod8 - mode
    dir1 dir2 dir3 dir4 dir5 dir6 dir7 dir8 dir9 dir10 dir11 dir12 dir13 dir14 dir15 dir16 dir17 dir18 dir19 dir20 dir21 dir22 dir23 dir24 dir25 dir26 dir27 dir28 dir29 dir30 dir31 dir32 dir33 dir34 dir35 dir36 dir37 dir38 dir39 dir40 dir41 dir42 dir43 dir44 dir45 dir46 dir47 dir48 dir49 dir50 dir51 dir52 dir53 dir54 dir55 dir56 dir57 dir58 dir59 dir60 dir61 dir62 dir63 dir64 dir65 dir66 dir67 dir68 dir69 dir70 - direction
    )
 (:init 
    (pointing sat1 dir17)
    (pointing sat2 dir53)
    (pointing sat3 dir47)
    (pointing sat4 dir12)
    (pointing sat5 dir51)
    (pointing sat6 dir70)
    (pointing sat7 dir9)
    (pointing sat8 dir46)
    (pointing sat9 dir31)
    (pointing sat10 dir56)
    (pointing sat11 dir8)
    (pointing sat12 dir36)
    (pointing sat13 dir62)
    (pointing sat14 dir36)
    (pointing sat15 dir49)
    (pointing sat16 dir41)
    (pointing sat17 dir16)
    (pointing sat18 dir20)
    (pointing sat19 dir51)
    (pointing sat20 dir59)
    (pointing sat21 dir8)
    (pointing sat22 dir20)
    (pointing sat23 dir22)
    (pointing sat24 dir5)
    (pointing sat25 dir41)
    (pointing sat26 dir41)
    (pointing sat27 dir17)
    (pointing sat28 dir61)
    (pointing sat29 dir61)
    (pointing sat30 dir45)
    (pointing sat31 dir62)
    (pointing sat32 dir69)
    (pointing sat33 dir58)
    (pointing sat34 dir15)
    (pointing sat35 dir11)
    (pointing sat36 dir13)
    (pointing sat37 dir32)
    (pointing sat38 dir68)
    (pointing sat39 dir66)
    (pointing sat40 dir18)
    (pointing sat41 dir27)
    (pointing sat42 dir56)
    (pointing sat43 dir15)
    (pointing sat44 dir12)
    (pointing sat45 dir45)
    (pointing sat46 dir56)
    (pointing sat47 dir43)
    (pointing sat48 dir48)
    (pointing sat49 dir43)
    (pointing sat50 dir28)
    (pointing sat51 dir23)
    (pointing sat52 dir38)
    (pointing sat53 dir19)
    (pointing sat54 dir65)
    (pointing sat55 dir11)
    (pointing sat56 dir44)
    (pointing sat57 dir28)
    (pointing sat58 dir13)
    (pointing sat59 dir28)
    (pointing sat60 dir20)
    (pointing sat61 dir63)
    (pointing sat62 dir8)
    (pointing sat63 dir12)
    (pointing sat64 dir61)
    (pointing sat65 dir3)
    (pointing sat66 dir5)
    (pointing sat67 dir2)
    (pointing sat68 dir59)
    (pointing sat69 dir34)
    (pointing sat70 dir37)
    (pointing sat71 dir27)
    (pointing sat72 dir2)
    (pointing sat73 dir53)
    (pointing sat74 dir18)
    (pointing sat75 dir61)
    (power_avail sat1)
    (power_avail sat2)
    (power_avail sat3)
    (power_avail sat4)
    (power_avail sat5)
    (power_avail sat6)
    (power_avail sat7)
    (power_avail sat8)
    (power_avail sat9)
    (power_avail sat10)
    (power_avail sat11)
    (power_avail sat12)
    (power_avail sat13)
    (power_avail sat14)
    (power_avail sat15)
    (power_avail sat16)
    (power_avail sat17)
    (power_avail sat18)
    (power_avail sat19)
    (power_avail sat20)
    (power_avail sat21)
    (power_avail sat22)
    (power_avail sat23)
    (power_avail sat24)
    (power_avail sat25)
    (power_avail sat26)
    (power_avail sat27)
    (power_avail sat28)
    (power_avail sat29)
    (power_avail sat30)
    (power_avail sat31)
    (power_avail sat32)
    (power_avail sat33)
    (power_avail sat34)
    (power_avail sat35)
    (power_avail sat36)
    (power_avail sat37)
    (power_avail sat38)
    (power_avail sat39)
    (power_avail sat40)
    (power_avail sat41)
    (power_avail sat42)
    (power_avail sat43)
    (power_avail sat44)
    (power_avail sat45)
    (power_avail sat46)
    (power_avail sat47)
    (power_avail sat48)
    (power_avail sat49)
    (power_avail sat50)
    (power_avail sat51)
    (power_avail sat52)
    (power_avail sat53)
    (power_avail sat54)
    (power_avail sat55)
    (power_avail sat56)
    (power_avail sat57)
    (power_avail sat58)
    (power_avail sat59)
    (power_avail sat60)
    (power_avail sat61)
    (power_avail sat62)
    (power_avail sat63)
    (power_avail sat64)
    (power_avail sat65)
    (power_avail sat66)
    (power_avail sat67)
    (power_avail sat68)
    (power_avail sat69)
    (power_avail sat70)
    (power_avail sat71)
    (power_avail sat72)
    (power_avail sat73)
    (power_avail sat74)
    (power_avail sat75)
    (calibration_target ins1 dir2)
    (calibration_target ins2 dir29)
    (calibration_target ins3 dir35)
    (calibration_target ins4 dir38)
    (calibration_target ins5 dir35)
    (calibration_target ins6 dir67)
    (calibration_target ins7 dir35)
    (calibration_target ins8 dir25)
    (calibration_target ins9 dir64)
    (calibration_target ins10 dir24)
    (calibration_target ins11 dir42)
    (calibration_target ins12 dir5)
    (calibration_target ins13 dir70)
    (calibration_target ins14 dir51)
    (calibration_target ins15 dir3)
    (calibration_target ins16 dir49)
    (calibration_target ins17 dir18)
    (calibration_target ins18 dir59)
    (calibration_target ins19 dir68)
    (calibration_target ins20 dir62)
    (calibration_target ins21 dir6)
    (calibration_target ins22 dir29)
    (calibration_target ins23 dir13)
    (calibration_target ins24 dir22)
    (calibration_target ins25 dir38)
    (calibration_target ins26 dir14)
    (calibration_target ins27 dir49)
    (calibration_target ins28 dir37)
    (calibration_target ins29 dir24)
    (calibration_target ins30 dir62)
    (calibration_target ins31 dir68)
    (calibration_target ins32 dir16)
    (calibration_target ins33 dir46)
    (calibration_target ins34 dir64)
    (calibration_target ins35 dir35)
    (calibration_target ins36 dir65)
    (calibration_target ins37 dir14)
    (calibration_target ins38 dir32)
    (calibration_target ins39 dir4)
    (calibration_target ins40 dir5)
    (calibration_target ins41 dir39)
    (calibration_target ins42 dir65)
    (calibration_target ins43 dir54)
    (calibration_target ins44 dir21)
    (calibration_target ins45 dir30)
    (calibration_target ins46 dir18)
    (calibration_target ins47 dir58)
    (calibration_target ins48 dir2)
    (calibration_target ins49 dir66)
    (calibration_target ins50 dir5)
    (calibration_target ins51 dir23)
    (calibration_target ins52 dir33)
    (calibration_target ins53 dir7)
    (calibration_target ins54 dir42)
    (calibration_target ins55 dir23)
    (calibration_target ins56 dir41)
    (calibration_target ins57 dir16)
    (calibration_target ins58 dir28)
    (calibration_target ins59 dir37)
    (calibration_target ins60 dir12)
    (calibration_target ins61 dir18)
    (calibration_target ins62 dir24)
    (calibration_target ins63 dir1)
    (calibration_target ins64 dir28)
    (calibration_target ins65 dir21)
    (calibration_target ins66 dir1)
    (calibration_target ins67 dir14)
    (calibration_target ins68 dir6)
    (calibration_target ins69 dir32)
    (calibration_target ins70 dir26)
    (calibration_target ins71 dir55)
    (calibration_target ins72 dir67)
    (calibration_target ins73 dir2)
    (calibration_target ins74 dir64)
    (calibration_target ins75 dir44)
    (calibration_target ins76 dir34)
    (calibration_target ins77 dir7)
    (calibration_target ins78 dir2)
    (calibration_target ins79 dir68)
    (calibration_target ins80 dir64)
    (calibration_target ins81 dir59)
    (calibration_target ins82 dir21)
    (calibration_target ins83 dir53)
    (calibration_target ins84 dir44)
    (calibration_target ins85 dir13)
    (calibration_target ins86 dir58)
    (calibration_target ins87 dir16)
    (calibration_target ins88 dir2)
    (calibration_target ins89 dir51)
    (calibration_target ins90 dir17)
    (calibration_target ins91 dir28)
    (calibration_target ins92 dir54)
    (calibration_target ins93 dir70)
    (calibration_target ins94 dir63)
    (calibration_target ins95 dir68)
    (calibration_target ins96 dir51)
    (calibration_target ins97 dir23)
    (calibration_target ins98 dir45)
    (calibration_target ins99 dir68)
    (calibration_target ins100 dir34)
    (calibration_target ins101 dir25)
    (calibration_target ins102 dir11)
    (calibration_target ins103 dir67)
    (calibration_target ins104 dir57)
    (calibration_target ins105 dir43)
    (calibration_target ins106 dir35)
    (calibration_target ins107 dir14)
    (calibration_target ins108 dir31)
    (calibration_target ins109 dir38)
    (calibration_target ins110 dir14)
    (calibration_target ins111 dir35)
    (calibration_target ins112 dir69)
    (calibration_target ins113 dir52)
    (calibration_target ins114 dir5)
    (calibration_target ins115 dir6)
    (calibration_target ins116 dir16)
    (calibration_target ins117 dir13)
    (calibration_target ins118 dir20)
    (calibration_target ins119 dir64)
    (calibration_target ins120 dir35)
    (calibration_target ins121 dir53)
    (calibration_target ins122 dir35)
    (calibration_target ins123 dir30)
    (calibration_target ins124 dir51)
    (calibration_target ins125 dir14)
    (on_board ins1 sat35)
    (on_board ins2 sat72)
    (on_board ins3 sat48)
    (on_board ins4 sat21)
    (on_board ins5 sat7)
    (on_board ins6 sat2)
    (on_board ins7 sat70)
    (on_board ins8 sat49)
    (on_board ins9 sat15)
    (on_board ins10 sat24)
    (on_board ins11 sat4)
    (on_board ins12 sat13)
    (on_board ins13 sat43)
    (on_board ins14 sat6)
    (on_board ins15 sat34)
    (on_board ins16 sat11)
    (on_board ins17 sat75)
    (on_board ins18 sat57)
    (on_board ins19 sat40)
    (on_board ins20 sat3)
    (on_board ins21 sat51)
    (on_board ins22 sat42)
    (on_board ins23 sat52)
    (on_board ins24 sat65)
    (on_board ins25 sat55)
    (on_board ins26 sat16)
    (on_board ins27 sat22)
    (on_board ins28 sat41)
    (on_board ins29 sat61)
    (on_board ins30 sat71)
    (on_board ins31 sat20)
    (on_board ins32 sat39)
    (on_board ins33 sat29)
    (on_board ins34 sat74)
    (on_board ins35 sat63)
    (on_board ins36 sat67)
    (on_board ins37 sat54)
    (on_board ins38 sat18)
    (on_board ins39 sat60)
    (on_board ins40 sat26)
    (on_board ins41 sat17)
    (on_board ins42 sat19)
    (on_board ins43 sat58)
    (on_board ins44 sat38)
    (on_board ins45 sat30)
    (on_board ins46 sat37)
    (on_board ins47 sat31)
    (on_board ins48 sat59)
    (on_board ins49 sat56)
    (on_board ins50 sat25)
    (on_board ins51 sat69)
    (on_board ins52 sat33)
    (on_board ins53 sat36)
    (on_board ins54 sat28)
    (on_board ins55 sat32)
    (on_board ins56 sat45)
    (on_board ins57 sat5)
    (on_board ins58 sat9)
    (on_board ins59 sat73)
    (on_board ins60 sat62)
    (on_board ins61 sat47)
    (on_board ins62 sat23)
    (on_board ins63 sat50)
    (on_board ins64 sat8)
    (on_board ins65 sat44)
    (on_board ins66 sat27)
    (on_board ins67 sat14)
    (on_board ins68 sat1)
    (on_board ins69 sat68)
    (on_board ins70 sat46)
    (on_board ins71 sat10)
    (on_board ins72 sat12)
    (on_board ins73 sat64)
    (on_board ins74 sat66)
    (on_board ins75 sat53)
    (on_board ins76 sat29)
    (on_board ins77 sat38)
    (on_board ins78 sat18)
    (on_board ins79 sat50)
    (on_board ins80 sat10)
    (on_board ins81 sat65)
    (on_board ins82 sat32)
    (on_board ins83 sat69)
    (on_board ins84 sat24)
    (on_board ins85 sat44)
    (on_board ins86 sat27)
    (on_board ins87 sat36)
    (on_board ins88 sat57)
    (on_board ins89 sat5)
    (on_board ins90 sat51)
    (on_board ins91 sat36)
    (on_board ins92 sat52)
    (on_board ins93 sat49)
    (on_board ins94 sat56)
    (on_board ins95 sat39)
    (on_board ins96 sat21)
    (on_board ins97 sat35)
    (on_board ins98 sat46)
    (on_board ins99 sat32)
    (on_board ins100 sat4)
    (on_board ins101 sat17)
    (on_board ins102 sat42)
    (on_board ins103 sat55)
    (on_board ins104 sat15)
    (on_board ins105 sat4)
    (on_board ins106 sat73)
    (on_board ins107 sat27)
    (on_board ins108 sat26)
    (on_board ins109 sat52)
    (on_board ins110 sat45)
    (on_board ins111 sat31)
    (on_board ins112 sat19)
    (on_board ins113 sat68)
    (on_board ins114 sat44)
    (on_board ins115 sat35)
    (on_board ins116 sat35)
    (on_board ins117 sat71)
    (on_board ins118 sat55)
    (on_board ins119 sat48)
    (on_board ins120 sat44)
    (on_board ins121 sat48)
    (on_board ins122 sat69)
    (on_board ins123 sat4)
    (on_board ins124 sat52)
    (on_board ins125 sat54)
    (supports ins15 mod1)
    (supports ins67 mod7)
    (supports ins23 mod7)
    (supports ins86 mod3)
    (supports ins72 mod6)
    (supports ins120 mod7)
    (supports ins80 mod8)
    (supports ins125 mod7)
    (supports ins3 mod3)
    (supports ins120 mod2)
    (supports ins11 mod8)
    (supports ins65 mod8)
    (supports ins109 mod1)
    (supports ins27 mod6)
    (supports ins114 mod7)
    (supports ins108 mod2)
    (supports ins96 mod1)
    (supports ins37 mod4)
    (supports ins93 mod7)
    (supports ins87 mod7)
    (supports ins111 mod1)
    (supports ins61 mod2)
    (supports ins16 mod4)
    (supports ins113 mod4)
    (supports ins116 mod6)
    (supports ins22 mod2)
    (supports ins1 mod3)
    (supports ins119 mod5)
    (supports ins63 mod6)
    (supports ins38 mod3)
    (supports ins81 mod6)
    (supports ins106 mod7)
    (supports ins101 mod5)
    (supports ins25 mod8)
    (supports ins88 mod3)
    (supports ins37 mod5)
    (supports ins43 mod8)
    (supports ins22 mod3)
    (supports ins93 mod6)
    (supports ins99 mod1)
    (supports ins37 mod3)
    (supports ins55 mod5)
    (supports ins74 mod2)
    (supports ins27 mod5)
    (supports ins53 mod1)
    (supports ins120 mod5)
    (supports ins27 mod2)
    (supports ins108 mod3)
    (supports ins21 mod1)
    (supports ins60 mod7)
    (supports ins75 mod2)
    (supports ins36 mod6)
    (supports ins41 mod2)
    (supports ins34 mod3)
    (supports ins63 mod2)
    (supports ins10 mod1)
    (supports ins77 mod3)
    (supports ins10 mod5)
    (supports ins6 mod8)
    (supports ins120 mod1)
    (supports ins40 mod7)
    (supports ins79 mod2)
    (supports ins4 mod6)
    (supports ins91 mod6)
    (supports ins28 mod2)
    (supports ins32 mod3)
    (supports ins19 mod1)
    (supports ins122 mod5)
    (supports ins5 mod6)
    (supports ins103 mod3)
    (supports ins99 mod8)
    (supports ins8 mod2)
    (supports ins35 mod4)
    (supports ins48 mod6)
    (supports ins52 mod5)
    (supports ins71 mod7)
    (supports ins51 mod3)
    (supports ins79 mod5)
    (supports ins19 mod3)
    (supports ins35 mod2)
    (supports ins41 mod1)
    (supports ins110 mod2)
    (supports ins76 mod7)
    (supports ins40 mod4)
    (supports ins73 mod7)
    (supports ins63 mod4)
    (supports ins83 mod3)
    (supports ins99 mod2)
    (supports ins120 mod3)
    (supports ins75 mod7)
    (supports ins82 mod8)
    (supports ins34 mod6)
    (supports ins65 mod6)
    (supports ins42 mod8)
    (supports ins48 mod7)
    (supports ins47 mod4)
    (supports ins8 mod8)
    (supports ins77 mod1)
    (supports ins80 mod2)
    (supports ins34 mod1)
    (supports ins46 mod5)
    (supports ins101 mod6)
    (supports ins42 mod3)
    (supports ins30 mod7)
    (supports ins42 mod6)
    (supports ins57 mod8)
    (supports ins90 mod8)
    (supports ins11 mod3)
    (supports ins60 mod6)
    (supports ins54 mod8)
    (supports ins44 mod1)
    (supports ins87 mod6)
    (supports ins7 mod5)
    (supports ins23 mod1)
    (supports ins47 mod3)
    (supports ins80 mod5)
    (supports ins8 mod7)
    (supports ins55 mod4)
    (supports ins50 mod1)
    (supports ins88 mod2)
    (supports ins110 mod6)
    (supports ins115 mod2)
    (supports ins100 mod4)
    (supports ins86 mod6)
    (supports ins94 mod3)
    (supports ins41 mod7)
    (supports ins25 mod3)
    (supports ins78 mod1)
    (supports ins66 mod2)
    (supports ins8 mod6)
    (supports ins56 mod4)
    (supports ins18 mod4)
    (supports ins76 mod3)
    (supports ins95 mod7)
    (supports ins56 mod8)
    (supports ins121 mod6)
    (supports ins49 mod4)
    (supports ins10 mod7)
    (supports ins113 mod7)
    (supports ins59 mod5)
    (supports ins13 mod8)
    (supports ins74 mod4)
    (supports ins57 mod7)
    (supports ins25 mod2)
    (supports ins53 mod6)
    (supports ins92 mod4)
    (supports ins110 mod1)
    (supports ins119 mod8)
    (supports ins27 mod3)
    (supports ins19 mod4)
    (supports ins64 mod1)
    (supports ins115 mod3)
    (supports ins68 mod4)
    (supports ins80 mod6)
    (supports ins23 mod5)
    (supports ins105 mod7)
    (supports ins73 mod2)
    (supports ins97 mod7)
    (supports ins109 mod2)
    (supports ins68 mod6)
    (supports ins58 mod5)
    (supports ins116 mod4)
    (supports ins72 mod8)
    (supports ins41 mod3)
    (supports ins65 mod3)
    (supports ins92 mod5)
    (supports ins62 mod3)
    (supports ins51 mod6)
    (supports ins123 mod8)
    (supports ins125 mod8)
    (supports ins82 mod6)
    (supports ins43 mod7)
    (supports ins95 mod8)
    (supports ins3 mod8)
    (supports ins62 mod6)
    (supports ins41 mod8)
    (supports ins124 mod2)
    (supports ins19 mod6)
    (supports ins74 mod8)
    (supports ins103 mod6)
    (supports ins98 mod1)
    (supports ins67 mod4)
    (supports ins72 mod7)
    (supports ins2 mod2)
    (supports ins66 mod4)
    (supports ins56 mod6)
    (supports ins107 mod4)
    (supports ins93 mod1)
    (supports ins88 mod4)
    (supports ins37 mod6)
    (supports ins89 mod6)
    (supports ins119 mod2)
    (supports ins101 mod8)
    (supports ins38 mod2)
    (supports ins47 mod6)
    (supports ins114 mod4)
    (supports ins63 mod5)
    (supports ins75 mod6)
    (supports ins105 mod5)
    (supports ins16 mod3)
    (supports ins8 mod3)
    (supports ins104 mod2)
    (supports ins52 mod4)
    (supports ins106 mod6)
    (supports ins12 mod3)
    (supports ins85 mod7)
    (supports ins125 mod5)
    (supports ins50 mod3)
    (supports ins87 mod8)
    (supports ins28 mod6)
    (supports ins73 mod3)
    (supports ins47 mod1)
    (supports ins117 mod2)
    (supports ins115 mod6)
    (supports ins74 mod1)
    (supports ins44 mod8)
    (supports ins118 mod7)
    (supports ins124 mod3)
    (supports ins23 mod8)
    (supports ins66 mod3)
    (supports ins4 mod1)
    (supports ins50 mod2)
    (supports ins95 mod2)
    (supports ins3 mod7)
    (supports ins26 mod7)
    (supports ins67 mod5)
    (supports ins86 mod7)
    (supports ins53 mod7)
    (supports ins14 mod3)
    (supports ins25 mod1)
    (supports ins79 mod1)
    (supports ins30 mod8)
    (supports ins16 mod6)
    (supports ins26 mod8)
    (supports ins120 mod8)
    (supports ins28 mod4)
    (supports ins14 mod5)
    (supports ins100 mod6)
    (supports ins33 mod7)
    (supports ins16 mod1)
    (supports ins66 mod6)
    (supports ins16 mod8)
    (supports ins63 mod8)
    (supports ins53 mod4)
    (supports ins92 mod8)
    (supports ins99 mod5)
    (supports ins89 mod1)
    (supports ins82 mod3)
    (supports ins52 mod2)
    (supports ins104 mod8)
    (supports ins105 mod4)
    (supports ins19 mod5)
    (supports ins17 mod4)
    (supports ins12 mod7)
    (supports ins39 mod7)
    (supports ins42 mod4)
    (supports ins98 mod3)
    (supports ins76 mod4)
    (supports ins2 mod6)
    (supports ins76 mod8)
    (supports ins74 mod5)
    (supports ins24 mod7)
    (supports ins2 mod1)
    (supports ins105 mod6)
    (supports ins84 mod5)
    (supports ins39 mod6)
    (supports ins31 mod1)
    (supports ins109 mod3)
    (supports ins120 mod6)
    (supports ins85 mod6)
    (supports ins9 mod3)
    (supports ins57 mod5)
    (supports ins2 mod7)
    (supports ins15 mod2)
    (supports ins28 mod3)
    (supports ins87 mod4)
    (supports ins113 mod3)
    (supports ins58 mod4)
    (supports ins104 mod5)
    (supports ins18 mod5)
    (supports ins83 mod2)
    (supports ins119 mod1)
    (supports ins6 mod3)
    (supports ins105 mod8)
    (supports ins107 mod8)
    (supports ins46 mod4)
    (supports ins62 mod5)
    (supports ins54 mod6)
    (supports ins38 mod7)
    (supports ins4 mod4)
    (supports ins71 mod3)
    (supports ins77 mod2)
    (supports ins11 mod2)
    (supports ins99 mod3)
    (supports ins4 mod7)
    (supports ins78 mod4)
    (supports ins82 mod5)
    (supports ins5 mod8)
    (supports ins114 mod6)
    (supports ins52 mod7)
    (supports ins16 mod7)
    (supports ins28 mod7)
    (supports ins5 mod3)
    (supports ins26 mod5)
    (supports ins33 mod4)
    (supports ins77 mod5)
    (supports ins49 mod7)
    (supports ins123 mod3)
    (supports ins20 mod8)
    (supports ins3 mod2)
    (supports ins75 mod1)
    (supports ins15 mod6)
    (supports ins44 mod3)
    (supports ins38 mod4)
    (supports ins25 mod4)
    (supports ins21 mod8)
    (supports ins36 mod4)
    (supports ins33 mod8)
    (supports ins28 mod1)
    (supports ins103 mod5)
    (supports ins32 mod2)
    (supports ins46 mod2)
    (supports ins14 mod2)
    (supports ins71 mod6)
    (supports ins97 mod2)
    (supports ins43 mod4)
    (supports ins116 mod1)
    (supports ins69 mod4)
    (supports ins70 mod8)
    (supports ins97 mod6)
    (supports ins48 mod2)
    (supports ins44 mod5)
    (supports ins49 mod5)
    (supports ins65 mod2)
    (supports ins75 mod5)
    (supports ins46 mod1)
    (supports ins47 mod2)
    (supports ins13 mod3)
    (supports ins54 mod1)
    (supports ins105 mod3)
    (supports ins108 mod8)
    (supports ins84 mod3)
    (supports ins67 mod8)
    (supports ins85 mod5)
    (supports ins118 mod3)
    (supports ins56 mod1)
    (supports ins8 mod1)
    (supports ins13 mod5)
    (supports ins106 mod4)
    (supports ins39 mod4)
    (supports ins39 mod8)
    (supports ins14 mod7)
    (supports ins69 mod7)
    (supports ins72 mod5)
    (supports ins32 mod6)
    (supports ins44 mod6)
    (supports ins85 mod2)
    (supports ins59 mod2)
    (supports ins89 mod5)
    (supports ins97 mod5)
    (supports ins4 mod2)
    (supports ins29 mod2)
    (supports ins124 mod1)
    (supports ins23 mod6)
    (supports ins76 mod5)
    (supports ins34 mod5)
    (supports ins86 mod1)
    (supports ins79 mod6)
    (supports ins6 mod1)
    (supports ins27 mod4)
    (supports ins45 mod7)
    (supports ins1 mod1)
    (supports ins1 mod5)
    (supports ins3 mod1)
    (supports ins7 mod2)
    (supports ins62 mod1)
    (supports ins18 mod8)
    (supports ins111 mod7)
    (supports ins122 mod6)
    (supports ins92 mod2)
    (supports ins92 mod6)
    (supports ins5 mod2)
    (supports ins61 mod4)
    (supports ins11 mod1)
    (supports ins54 mod7)
    (supports ins88 mod8)
    (supports ins31 mod8)
    (supports ins81 mod4)
    (supports ins79 mod3)
    (supports ins2 mod5)
    (supports ins71 mod4)
    (supports ins113 mod2)
    (supports ins55 mod1)
    (supports ins13 mod2)
    (supports ins31 mod6)
    (supports ins15 mod5)
    (supports ins51 mod1)
    (supports ins44 mod7)
    (supports ins54 mod3)
    (supports ins21 mod3)
    (supports ins52 mod6)
    (supports ins117 mod1)
    (supports ins104 mod6)
    (supports ins6 mod6)
    (supports ins106 mod2)
    (supports ins114 mod5)
    (supports ins105 mod2)
    (supports ins94 mod5)
    (supports ins31 mod3)
    (supports ins49 mod1)
    (supports ins46 mod3)
    (supports ins84 mod8)
    (supports ins66 mod7)
    (supports ins45 mod3)
    (supports ins50 mod5)
    (supports ins9 mod8)
    (supports ins47 mod8)
    (supports ins108 mod1)
    (supports ins16 mod2)
    (supports ins37 mod1)
    (supports ins27 mod7)
    (supports ins31 mod2)
    (supports ins125 mod4)
    (supports ins94 mod4)
    (supports ins64 mod8)
    (supports ins7 mod6)
    (supports ins107 mod7)
    (supports ins7 mod7)
    (supports ins52 mod8)
    (supports ins107 mod5)
    (supports ins91 mod5)
    (supports ins14 mod8)
    (supports ins48 mod4)
    (supports ins53 mod3)
    (supports ins17 mod1)
    (supports ins10 mod6)
    (supports ins117 mod6)
    (supports ins78 mod2)
    (supports ins96 mod7)
    (supports ins113 mod5)
    (supports ins64 mod7)
    (supports ins7 mod4)
    (supports ins97 mod4)
    (supports ins114 mod1)
    (supports ins125 mod1)
    (supports ins33 mod1)
    (supports ins60 mod3)
    (supports ins86 mod8)
    (supports ins120 mod4)
    (supports ins101 mod7)
    (supports ins123 mod5)
    (supports ins90 mod7)
    (supports ins79 mod4)
    (supports ins82 mod4)
    (supports ins5 mod1)
    (supports ins94 mod6)
    (supports ins7 mod1)
    (supports ins23 mod3)
    (supports ins34 mod4)
    (supports ins92 mod1)
    (supports ins26 mod6)
    (supports ins26 mod2)
    (supports ins35 mod8)
    (supports ins39 mod1)
    (supports ins111 mod3)
    (supports ins38 mod5)
    (supports ins56 mod2)
    (supports ins45 mod6)
    (supports ins113 mod6)
    (supports ins26 mod3)
    (supports ins69 mod2)
    (supports ins15 mod4)
    (supports ins118 mod5)
    (supports ins116 mod5)
    (supports ins61 mod7)
    (supports ins3 mod5)
    (supports ins118 mod6)
    (supports ins80 mod7)
    (supports ins40 mod3)
    (supports ins118 mod8)
    (supports ins104 mod4)
    (supports ins19 mod7)
    (supports ins3 mod4)
    (supports ins87 mod5)
    (supports ins32 mod4)
    (supports ins102 mod5)
    (supports ins96 mod6)
    (supports ins31 mod5)
    (supports ins100 mod2)
    (supports ins22 mod5)
    (supports ins101 mod4)
    (supports ins19 mod2)
    (supports ins5 mod7)
    (supports ins1 mod7)
    (supports ins18 mod2)
    (supports ins24 mod3)
    (supports ins58 mod2)
    (supports ins96 mod4)
    (supports ins40 mod6)
    (supports ins48 mod1)
    (supports ins48 mod5)
    (supports ins102 mod1)
    (supports ins61 mod8)
    (supports ins108 mod4)
    (supports ins111 mod2)
    (supports ins37 mod2)
    (supports ins110 mod8)
    (supports ins29 mod7)
    (supports ins71 mod1)
    (supports ins116 mod3)
    (supports ins18 mod7)
    (supports ins86 mod2)
    (supports ins56 mod5)
    (supports ins22 mod8)
    (supports ins110 mod5)
    (supports ins68 mod5)
    (supports ins83 mod8)
    (supports ins20 mod2)
    (supports ins29 mod8)
    (supports ins63 mod3)
    (supports ins29 mod6)
    (supports ins59 mod4)
    (supports ins57 mod3)
    (supports ins91 mod4)
    (supports ins24 mod8)
    (supports ins95 mod6)
    (supports ins60 mod5)
    (supports ins18 mod1)
    (supports ins51 mod7)
    (supports ins58 mod7)
    (supports ins52 mod1)
    (supports ins116 mod7)
    (supports ins125 mod2)
    (supports ins36 mod3)
    (supports ins85 mod4)
    (supports ins77 mod6)
    (supports ins46 mod7)
    (supports ins119 mod4)
    (supports ins28 mod8)
    (supports ins60 mod1)
    (supports ins78 mod6)
    (supports ins53 mod8)
    (supports ins20 mod7)
    (supports ins106 mod8)
    (supports ins109 mod5)
    (supports ins73 mod1)
    (supports ins66 mod5)
    (supports ins104 mod3)
    (supports ins112 mod4))
 (:goal  (and (pointing sat2 dir4)
   (pointing sat3 dir54)
   (pointing sat6 dir59)
   (pointing sat7 dir6)
   (pointing sat11 dir58)
   (pointing sat13 dir15)
   (pointing sat14 dir39)
   (pointing sat19 dir58)
   (pointing sat20 dir60)
   (pointing sat22 dir12)
   (pointing sat23 dir13)
   (pointing sat27 dir62)
   (pointing sat28 dir50)
   (pointing sat29 dir41)
   (pointing sat32 dir21)
   (pointing sat33 dir22)
   (pointing sat36 dir66)
   (pointing sat40 dir19)
   (pointing sat41 dir54)
   (pointing sat42 dir17)
   (pointing sat43 dir67)
   (pointing sat44 dir45)
   (pointing sat46 dir50)
   (pointing sat47 dir57)
   (pointing sat49 dir51)
   (pointing sat52 dir34)
   (pointing sat53 dir1)
   (pointing sat61 dir41)
   (pointing sat63 dir37)
   (pointing sat71 dir69)
   (pointing sat73 dir44)
   (have_image dir56 mod2)
   (have_image dir67 mod2)
   (have_image dir16 mod4)
   (have_image dir52 mod8)
   (have_image dir2 mod5)
   (have_image dir7 mod2)
   (have_image dir26 mod2)
   (have_image dir57 mod3)
   (have_image dir14 mod3)
   (have_image dir59 mod2)
   (have_image dir7 mod8)
   (have_image dir62 mod4)
   (have_image dir23 mod7)
   (have_image dir4 mod8)
   (have_image dir66 mod1)
   (have_image dir49 mod1)
   (have_image dir68 mod8)
   (have_image dir34 mod5)
   (have_image dir13 mod3)
   (have_image dir43 mod4)
   (have_image dir41 mod2)
   (have_image dir10 mod7)
   (have_image dir49 mod7)
   (have_image dir60 mod8)
   (have_image dir51 mod2)
   (have_image dir42 mod4)
   (have_image dir18 mod6)
   (have_image dir56 mod1)
   (have_image dir70 mod6)
   (have_image dir41 mod8)
   (have_image dir53 mod1)
   (have_image dir40 mod7)
   (have_image dir70 mod1)
   (have_image dir57 mod6)
   (have_image dir8 mod1)
   (have_image dir30 mod7)
   (have_image dir27 mod7)
   (have_image dir17 mod7)
   (have_image dir66 mod2)
   (have_image dir12 mod6)
   (have_image dir23 mod4)
   (have_image dir61 mod5)
   (have_image dir41 mod1)
   (have_image dir57 mod4)
   (have_image dir59 mod7)
   (have_image dir28 mod7)
   (have_image dir35 mod1)
   (have_image dir52 mod7)
   (have_image dir9 mod4)
   (have_image dir6 mod6)
   (have_image dir40 mod8)
   (have_image dir56 mod4)
   (have_image dir69 mod4)
   (have_image dir48 mod1)
   (have_image dir20 mod6)
   (have_image dir12 mod2)
   (have_image dir3 mod8)
   (have_image dir46 mod3)
   (have_image dir25 mod2)
   (have_image dir55 mod1)
   (have_image dir16 mod1)
   (have_image dir55 mod4)
   (have_image dir47 mod4)
   (have_image dir12 mod5)
   (have_image dir41 mod7)
   (have_image dir25 mod8)
   (have_image dir5 mod2)
   (have_image dir24 mod8)
   (have_image dir44 mod6)
   (have_image dir59 mod1)
   (have_image dir37 mod6)
   (have_image dir17 mod5)
   (have_image dir29 mod3)
   (have_image dir15 mod3)
   (have_image dir68 mod7)
   (have_image dir21 mod2)
   (have_image dir60 mod5)
   (have_image dir70 mod8)
   (have_image dir62 mod5)
   (have_image dir56 mod5)
   (have_image dir30 mod8)
   (have_image dir45 mod2)
   (have_image dir9 mod1)
   (have_image dir31 mod1)
   (have_image dir51 mod1)
   (have_image dir65 mod8)
   (have_image dir4 mod1)
   (have_image dir4 mod3)
   (have_image dir3 mod2)
   (have_image dir31 mod6)
   (have_image dir56 mod8)
   (have_image dir65 mod1)
   (have_image dir41 mod5)
   (have_image dir29 mod8)
   (have_image dir25 mod3)
   (have_image dir10 mod4)
   (have_image dir24 mod3)
   (have_image dir34 mod1)
   (have_image dir10 mod6)
   (have_image dir39 mod7)
   (have_image dir1 mod2)
   (have_image dir29 mod4)
   (have_image dir41 mod3)
   (have_image dir40 mod6)
   (have_image dir63 mod5)
   (have_image dir46 mod5)
   (have_image dir9 mod8)
   (have_image dir67 mod4)
   (have_image dir5 mod6)
   (have_image dir28 mod4)
   (have_image dir32 mod6)
   (have_image dir57 mod8)
   (have_image dir53 mod2)
   (have_image dir17 mod6)
   (have_image dir46 mod4)
   (have_image dir59 mod4)
   (have_image dir67 mod8)
   (have_image dir28 mod2)
   (have_image dir12 mod8)
   (have_image dir7 mod6)
   (have_image dir31 mod3)
   (have_image dir67 mod5)
   (have_image dir51 mod7)
   (have_image dir44 mod7)
   (have_image dir63 mod6)
   (have_image dir43 mod1)
   (have_image dir42 mod5)
   (have_image dir38 mod7)
   (have_image dir68 mod6)
   (have_image dir62 mod3)
   (have_image dir36 mod8)
   (have_image dir61 mod3)
   (have_image dir46 mod7)
   (have_image dir1 mod1)
   (have_image dir11 mod4)
   (have_image dir4 mod6)
   (have_image dir33 mod8)
   (have_image dir49 mod6)
   (have_image dir38 mod4)
   (have_image dir13 mod6)
   (have_image dir31 mod2)
   (have_image dir36 mod7)
   (have_image dir63 mod2)
   (have_image dir68 mod3)
   (have_image dir10 mod1)
   (have_image dir18 mod5)
   (have_image dir63 mod4)
   (have_image dir29 mod6)
   (have_image dir60 mod1)
   (have_image dir53 mod3)
   (have_image dir15 mod8)
   (have_image dir12 mod4)
   (have_image dir26 mod8)
   (have_image dir20 mod3)
   (have_image dir21 mod6)
   (have_image dir43 mod8)
   (have_image dir2 mod7)
   (have_image dir45 mod8)
   (have_image dir70 mod2)
   (have_image dir24 mod2)
   (have_image dir33 mod6)
   (have_image dir37 mod8)
   (have_image dir65 mod2)
   (have_image dir44 mod5)
   (have_image dir50 mod1)
   (have_image dir69 mod3)
   (have_image dir15 mod6)
   (have_image dir36 mod6)
   (have_image dir45 mod3)
   (have_image dir44 mod3)
   (have_image dir14 mod6)
   (have_image dir3 mod3)
   (have_image dir2 mod6)
   (have_image dir9 mod3)
   (have_image dir37 mod3)
   (have_image dir36 mod4)
   (have_image dir43 mod6)
   (have_image dir60 mod3)
   (have_image dir49 mod8)
   (have_image dir49 mod3)
   (have_image dir10 mod3)
   (have_image dir47 mod5)
   (have_image dir35 mod2)
   (have_image dir23 mod1)
   (have_image dir58 mod6)
   (have_image dir51 mod4)
   (have_image dir54 mod7)
   (have_image dir46 mod1)
   (have_image dir38 mod5)
   (have_image dir29 mod5)
   (have_image dir21 mod4)
   (have_image dir12 mod7)
   (have_image dir45 mod7)
   (have_image dir58 mod4)
   (have_image dir34 mod7)
   (have_image dir11 mod6)
   (have_image dir64 mod8)
   (have_image dir54 mod2)
   (have_image dir48 mod7)
   (have_image dir30 mod2)
   (have_image dir47 mod8)
   (have_image dir16 mod3)
   (have_image dir41 mod4)
   (have_image dir9 mod5)
   (have_image dir6 mod7)
   (have_image dir67 mod7)
   (have_image dir22 mod5)
   (have_image dir70 mod3)
   (have_image dir27 mod8)
   (have_image dir15 mod4)
   (have_image dir8 mod8))))