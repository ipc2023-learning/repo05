;; satellites=6, instruments=11, modes=2, directions=7, out_folder=testing/easy, instance_id=15, seed=1021

(define (problem satellite-15)
 (:domain satellite)
 (:objects 
    sat1 sat2 sat3 sat4 sat5 sat6 - satellite
    ins1 ins2 ins3 ins4 ins5 ins6 ins7 ins8 ins9 ins10 ins11 - instrument
    mod1 mod2 - mode
    dir1 dir2 dir3 dir4 dir5 dir6 dir7 - direction
    )
 (:init 
    (pointing sat1 dir5)
    (pointing sat2 dir5)
    (pointing sat3 dir2)
    (pointing sat4 dir5)
    (pointing sat5 dir2)
    (pointing sat6 dir3)
    (power_avail sat1)
    (power_avail sat2)
    (power_avail sat3)
    (power_avail sat4)
    (power_avail sat5)
    (power_avail sat6)
    (calibration_target ins1 dir5)
    (calibration_target ins2 dir4)
    (calibration_target ins3 dir5)
    (calibration_target ins4 dir4)
    (calibration_target ins5 dir6)
    (calibration_target ins6 dir4)
    (calibration_target ins7 dir3)
    (calibration_target ins8 dir1)
    (calibration_target ins9 dir1)
    (calibration_target ins10 dir7)
    (calibration_target ins11 dir4)
    (on_board ins1 sat5)
    (on_board ins2 sat1)
    (on_board ins3 sat6)
    (on_board ins4 sat4)
    (on_board ins5 sat2)
    (on_board ins6 sat3)
    (on_board ins7 sat3)
    (on_board ins8 sat6)
    (on_board ins9 sat1)
    (on_board ins10 sat2)
    (on_board ins11 sat5)
    (supports ins10 mod1)
    (supports ins1 mod2)
    (supports ins5 mod1)
    (supports ins3 mod2)
    (supports ins5 mod2)
    (supports ins10 mod2)
    (supports ins3 mod1)
    (supports ins7 mod1)
    (supports ins2 mod1)
    (supports ins6 mod1)
    (supports ins1 mod1)
    (supports ins8 mod1)
    (supports ins7 mod2)
    (supports ins9 mod2)
    (supports ins4 mod1)
    (supports ins9 mod1)
    (supports ins11 mod1))
 (:goal  (and (pointing sat3 dir7)
   (pointing sat4 dir6)
   (have_image dir5 mod2)
   (have_image dir7 mod1)
   (have_image dir2 mod2)
   (have_image dir7 mod2)
   (have_image dir2 mod1)
   (have_image dir3 mod2))))