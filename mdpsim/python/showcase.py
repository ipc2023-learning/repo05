import mdpsim as m
import os

parsed = False


if __name__ == "__main__":

    my_path = os.path.dirname(os.path.abspath(__file__))
    tt_path = os.path.join(my_path, '..', 'examples', 'domain.pddl')
    m.parse_file(tt_path)
    tt_path = os.path.join(my_path, '..', 'examples', 'p01.pddl')
    m.parse_file(tt_path)
    problem = m.get_problems()['satellite-01']
    print(problem.ground_actions)


