#include "codexion.h"

void	ft_cleanup(t_sim *sim)
{
	int	i;

	i = 0;
	if (sim->dongles)
	{
		while (i < sim->number_of_coders)
		{
			pthread_mutex_destroy(&sim->dongles[i].mutex);
			pthread_cond_destroy(&sim->dongles[i].cond);
			pq_free(&sim->dongles[i].queue);
			i++;
		}
		free(sim->dongles);
	}
	if (sim->coders)
		free(sim->coders);
	pthread_mutex_destroy(&sim->print_mutex);
	pthread_mutex_destroy(&sim->sim_mutex);
    pthread_mutex_destroy(&sim->monitor_mutex);
    pthread_cond_destroy(&sim->monitor_cond);
}