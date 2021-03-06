// Code generated by goctl. DO NOT EDIT.
package handler

import (
	"net/http"

	"study/micro/dtm/greet/internal/svc"

	"github.com/tal-tech/go-zero/rest"
)

func RegisterHandlers(server *rest.Server, serverCtx *svc.ServiceContext) {
	server.AddRoutes(
		[]rest.Route{
			{
				Method:  http.MethodGet,
				Path:    "/from/:name",
				Handler: GreetHandler(serverCtx),
			},
		},
	)
}
