#include "codexion.h"

static int	launch_threads(t_sim *sim)
{
	int	i;

	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim) != 0)
		return (0);
	i = 0;
	while (i < sim->number_of_coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL,
				coder_routine, &sim->coders[i]) != 0)
			return (0);
		i++;
	}
	return (1);
}

static void	join_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_coders)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
	pthread_mutex_lock(&sim->monitor_mutex);
	pthread_cond_signal(&sim->monitor_cond);
	pthread_mutex_unlock(&sim->monitor_mutex);
	pthread_join(sim->monitor, NULL);
}

int	main(int argc, char **argv)
{
	t_sim	sim;

	if (!ft_parsing(argc, argv, &sim))
		return (1);
	if (!ft_init(&sim))
	{
		ft_cleanup(&sim);
		return (1);
	}
	if (!launch_threads(&sim))
	{
		ft_cleanup(&sim);
		return (1);
	}
	join_threads(&sim);
	ft_cleanup(&sim);
	return (0);
}