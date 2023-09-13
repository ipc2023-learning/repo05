;;;  Authors: Michael Littman and David Weissman  ;;;

(define (domain exploding-blocks-world-pre)
  (:requirements  :strips :typing :probabilistic-effects
                  :negative-preconditions :universal-preconditions
                  :conditional-effects :equality)
  (:types block)
  (:predicates (detonated ?b - block)
               (destroyed ?b - block)
               (destroyed-table)
               (holding ?b-block)
               (on-top-of ?b1 - block ?b2 - block)
               (on-top-of-table ?b-block))

  (:action pick-up-block
           :parameters (?b - block)
           :precondition (and (not (destroyed ?b))
                              (not (destroyed-table))
                              (forall (?x - block) (not (on-top-of ?x ?b))) ; no block sits on this one
                              (forall (?x - block) (not (holding ?x)))) ; no block is currently being held
           :effect (and (holding ?b)
                        (forall (?x - block) (not (on-top-of ?b ?x))))) ; this block no longer sits on top any other

  (:action put-down-block-on-table ; put ?b on table (risks ?b exploding and destroying table)
           :parameters (?b - block)
           :precondition (and (holding ?b)
                              (not (destroyed-table)))
           :effect (and (not (holding ?b))
                        (on-top-of-table ?b)
                        (when (not (detonated ?b))
                              (probabilistic .3 (and (detonated ?b)
                                                     (destroyed-table))))))

  (:action put-down-block-on-block ; put ?b1 down into ?b2 (risks ?b1 exploding and destroying ?b2)
           :parameters (?b1 - block ?b2 - block)
           :precondition (and (holding ?b1)
                              (not (destroyed ?b2))
                              (not (destroyed-table))
                              (not (= ?b1 ?b2)) ; probably fixes "block on itself" problem (?)
                              (forall (?x - block) (not (on-top-of ?x ?b2)))) ; no block sits on this one
           :effect (and (not (holding ?b1))
                        (when (not (detonated ?b1))
                              (probabilistic .3 (and (detonated ?b1) ; KABOOM
                                                     (destroyed ?b2)
                                                     (when (on-top-of-table ?b2)
                                                           (and (on-top-of-table ?b1)
                                                                (not (on-top-of-table ?b2))))
                                                     (forall (?x - block) ; ?b2 got died, so ?b1 now goes on top of whatever ?b2 sat on before
                                                             (when (on-top-of ?b2 ?x)
                                                                   (and (not (on-top-of ?b2 ?x))
                                                                        (on-top-of ?b1 ?x)))))
                                             .7 (on-top-of ?b1 ?b2))) ; stacked okay, phew
                        (when (detonated ?b1) (on-top-of ?b1 ?b2)))))

(define (problem exploding-blocks-prob-pre)
  (:domain exploding-blocks-world-pre)
  (:objects b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 b10 - block)
  (:init (on-top-of b0 b1)
         (on-top-of b1 b3)
         (on-top-of b3 b7)
         (on-top-of-table b7)
         (on-top-of b2 b4)
         (on-top-of-table b4)
         (on-top-of-table b5)
         (on-top-of b6 b8)
         (on-top-of-table b8)
         (on-top-of-table b9)
         (on-top-of-table b10))
  (:goal (and (on-top-of b7 b3)
              (on-top-of b0 b1)
              (on-top-of-table b1)
              (on-top-of b3 b5)
              (on-top-of-table b5)
              (on-top-of b6 b2))))
