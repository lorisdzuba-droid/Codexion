#include "codexion.h"

static int	is_positive_int(char *str)
{
	int	i;

	i = 0;
	if (!str || str[0] == '\0')
		return (0);
	while (str[i])
	{
		if (str[i] < '0' || str[i] > '9')
			return (0);
		i++;
	}
	return (1);
}

static int	parse_scheduler(char *str, t_scheduler *scheduler)
{
	if (strcmp(str, "fifo") == 0)
		*scheduler = FIFO;
	else if (strcmp(str, "edf") == 0)
		*scheduler = EDF;
	else
		return (0);
	return (1);
}

static int	fill_sim(char **argv, t_sim *sim)
{
	sim->number_of_coders			= atoi(argv[1]);
	sim->time_to_burnout			= (long long)atoi(argv[2]);
	sim->time_to_compile			= (long long)atoi(argv[3]);
	sim->time_to_debug				= (long long)atoi(argv[4]);
	sim->time_to_refactor			= (long long)atoi(argv[5]);
	sim->number_of_compiles_required	= atoi(argv[6]);
	sim->dongle_cooldown			= (long long)atoi(argv[7]);
	if (!parse_scheduler(argv[8], &sim->scheduler))
		return (0);
	if (sim->number_of_coders < 1)
		return (0);
	if (sim->time_to_burnout < 0 || sim->time_to_compile < 0
		|| sim->time_to_debug < 0 || sim->time_to_refactor < 0)
		return (0);
	if (sim->number_of_compiles_required < 1)
		return (0);
	if (sim->dongle_cooldown < 0)
		return (0);
	return (1);
}

int	ft_parsing(int argc, char **argv, t_sim *sim)
{
	int	i;

	if (argc != 9)
	{
		fprintf(stderr, "Error: expected 8 arguments\n");
		fprintf(stderr, "Usage: %s number_of_coders time_to_burnout "
			"time_to_compile time_to_debug time_to_refactor "
			"number_of_compiles_required dongle_cooldown scheduler\n", argv[0]);
		return (0);
	}
	i = 1;
	while (i <= 7)
	{
		if (!is_positive_int(argv[i]))
		{
			fprintf(stderr, "Error: argument %d must be a non-negative integer\n", i);
			return (0);
		}
		i++;
	}
	if (!fill_sim(argv, sim))
	{
		fprintf(stderr, "Error: invalid argument values\n");
		return (0);
	}
	return (1);
}