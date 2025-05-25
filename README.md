# MC937-animation
Animation to MC937

Objetivo ler 3 OBJ (Novos objetos podem ser baixados do https://github.com/alecjacobson/common-3d-test-models)

*casos*:
- Se o objeto não está sofrendo colisão, ele estará sofrendo apenas as forças do ambiente
  - Gravidade F = mg
  - Arraste do Ar F = - kv^2
  - Vento com direção aleatória sobre o plano xy $$f_{xy} = randon(0, 1)x + randon(0,1)y$$

- Se o objeto sofre colisão, além das forças acima citadas, ele sofre forças de contato
  - Atrito $$F_{at} = N.\mu$$
  - Deformação de ricochete
  - Deformação do objeto (Elasticidade), cada face irá ser regida pela $$F = -k\Delta x$$

**Cena 1** - com o segundo objeto, simular uma queda no chão e ricochetear

```bash
./build/scene1 ./obj/homer.obj ./obj/homer.obj ./obj/homer.obj
```

**Cena 2** Simular um pano caindo sobre um dos objetos

```bash
./build/scene1 ./obj/homer.obj ./obj/homer.obj ./obj/homer.obj
```
**Cena 3** - N objetos são posicionados aleatoriamente entorno do centro e estão sujeitos a uma força de retoção (tornado), deixando evidente o impacto entre os n objetos

```bash
./build/scene3 ./OBJ/homer.obj 1000
```

Cada cena tem seus frame salvos na pasta *frames* e em cada respectiva cena com nome *scene*
