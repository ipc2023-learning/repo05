(define (problem bomb-and-toilet)
  (:domain bomb-and-toilet)
  (:requirements :negative-preconditions)
  (:objects package1 package2)
  (:init (probabilistic 0.5 (bomb-in-package package1)
			0.5 (bomb-in-package package2)))
  (:goal (and (bomb-defused) (not (toilet-clogged)))))