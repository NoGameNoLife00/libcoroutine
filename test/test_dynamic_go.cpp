#include <libcoro.h>
#include <string>

static const int M = 10;
size_t dynamic_go_count = 0;
std::array<std::array<std::array<int32_t, M>, M>, 3> dynamic_cells;
void test_dynamic_go() {
    auto co_star = [](int j) -> libcoro::Future<int> {
        for (int i = 0; i < M; i++) {
            go[=]() -> libcoro::Generator<int> {
                for (int k = 0; k < M; k++) {
                    ++dynamic_cells[j][i][k];
                    ++dynamic_go_count;
                    printf("%d  %d  %d\n", j, i, k);
                    co_yield k;
                }
                co_return M;
            };

            co_yield i;
        }
        co_return M;
    };

    go co_star(0);
    go co_star(1);
    go co_star(2);

    libcoro::ThisScheduler()->RunUtilNoTask();

    printf("dynamic_go_count = %zu\n", dynamic_go_count);
    for (auto &j : dynamic_cells) {
        for (auto& i: j) {
            for (auto k : i) {
                printf("%d", k);
            }
            printf("\n");
        }
        printf("\n");
    }
}

int main()
{
    setbuf(stdout, nullptr); // debug
    test_dynamic_go();
    return 0;
}
